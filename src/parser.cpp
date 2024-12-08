#include "parser.h"
#include "lexer.h"
#include <algorithm>

Parser::Parser(const std::string &filename)
    : lexer_(filename), current_parent_(nullptr) {}

ASTNode *root = new ASTNode("Program");

void Parser::Initalize() {
  current_parent_ = root;
  scope_stack_.push_back(root);
}

ASTNode *Parser::parse() {
  std::pair<TokenType, std::string> token;

  do {
    token = lexer_.getNextToken();

    switch (token.first) {
    case TokenType::KEYWORD:
      if (token.second == "if") {
        ASTNode *if_node = new ASTNode("STATEMENT", "if");

        Condition condition = parseCondition();

        if (condition.error) {
          ASTNode *error_node = new ASTNode("ERROR", "error");
          root->add_child(error_node);
          return nullptr;
        } else {
          ASTNode *condition_node = new ASTNode("CONDITION", "");
          ASTNode *left_condition_node = new ASTNode(
              tokenTypeToString(condition.left.first), condition.left.second);
          ASTNode *operator_condition_node = new ASTNode(
              tokenTypeToString(condition.op.first), condition.op.second);
          ASTNode *right_condition_node = new ASTNode(
              tokenTypeToString(condition.right.first), condition.right.second);

          condition_node->add_child(left_condition_node);
          condition_node->add_child(operator_condition_node);
          condition_node->add_child(right_condition_node);
          if_node->add_child(condition_node);

          ASTNode *codeBlock_node = new ASTNode("CODE_BLOCK", "");
          if_node->add_child(codeBlock_node);
          current_parent_->add_child(if_node);
          switchParentNode(codeBlock_node);
        }
      } else if (token.second == "else if") {
        ASTNode *elseif_node = new ASTNode("STATEMENT", "else if");

        Condition condition = parseCondition();

        if (condition.error) {
          ASTNode *error_node = new ASTNode("ERROR", "error");
          root->add_child(error_node);
          return nullptr;
        } else {
          ASTNode *condition_node = new ASTNode("CONDITION", "");
          ASTNode *left_condition_node = new ASTNode(
              tokenTypeToString(condition.left.first), condition.left.second);
          ASTNode *operator_condition_node = new ASTNode(
              tokenTypeToString(condition.op.first), condition.op.second);
          ASTNode *right_condition_node = new ASTNode(
              tokenTypeToString(condition.right.first), condition.right.second);
          ASTNode *codeBlock_node = new ASTNode("CODE_BLOCK", "");

          condition_node->add_child(left_condition_node);
          condition_node->add_child(operator_condition_node);
          condition_node->add_child(right_condition_node);
          elseif_node->add_child(condition_node);
          elseif_node->add_child(codeBlock_node);

          current_parent_->add_child(elseif_node);
          switchParentNode(codeBlock_node);
        }
      } else if (token.second == "else") {
        token = lexer_.getNextToken();
        if (token.first == TokenType::CURLY_PAREN && token.second == "{") {
          ASTNode *else_node = new ASTNode("STATEMENT", "else");
          ASTNode *codeBlock_node = new ASTNode("CODE_BLOCK", "");

          else_node->add_child(codeBlock_node);
          current_parent_->add_child(else_node);
          switchParentNode(codeBlock_node);
          parse();
        }
      } else if (token.second == "while") {
        ASTNode *while_node = new ASTNode("STATEMENT", "while");

        Condition condition = parseCondition();

        if (condition.error) {
          ASTNode *error_node = new ASTNode("ERROR", "error");
          root->add_child(error_node);
          return nullptr;
        } else {
          ASTNode *condition_node = new ASTNode("CONDITION", "");
          ASTNode *codeBlock_node = new ASTNode("CODE_BLOCK", "");

          while_node->add_child(condition_node);
          while_node->add_child(codeBlock_node);

          current_parent_->add_child(while_node);
          switchParentNode(codeBlock_node);
          parse();
        }
      } else if (token.second == "for") {
        ASTNode *for_node = new ASTNode("STATEMENT", "for");

        ForLoopCondition condition = parseForLoopCondition();

        if (condition.error) {
          std::cerr << "" << std::endl;
          ASTNode *error_node = new ASTNode("ERROR", "error");
          root->add_child(error_node);
          return nullptr;
        } else {
          ASTNode *condition_node = new ASTNode("CONDITON", "");
          ASTNode *int_node = new ASTNode("INT", condition.cInt.second);
          ASTNode *i1_node = new ASTNode("IDENTIFIER", condition.i.second);
          ASTNode *assignment_node = new ASTNode("ASSIGNMENT", "");
          ASTNode *nl_node = new ASTNode(tokenTypeToString(condition.nl.first),
                                         condition.nl.second);
          ASTNode *i2_node = new ASTNode("IDENTIFIER", condition.i2.second);
          ASTNode *ro_node =
              new ASTNode("RELATIONAL_OPERATOR", condition.ro.second);
          ASTNode *len_node = new ASTNode(
              tokenTypeToString(condition.len.first), condition.len.second);
          ASTNode *i3_node = new ASTNode("IDENTIFIER", condition.i3.second);
          ASTNode *uao_node =
              new ASTNode("UNARY_ARITHMETIC_OPERATOR", condition.uao.second);
          ASTNode *codeBlock_node = new ASTNode("CODE_BLOCK", "");

          condition_node->add_child(int_node);
          condition_node->add_child(i1_node);
          condition_node->add_child(assignment_node);
          assignment_node->add_child(nl_node);
          condition_node->add_child(i2_node);
          condition_node->add_child(ro_node);
          condition_node->add_child(len_node);
          condition_node->add_child(i3_node);
          condition_node->add_child(uao_node);
          for_node->add_child(condition_node);
          for_node->add_child(codeBlock_node);

          current_parent_->add_child(for_node);
          switchParentNode(codeBlock_node);
          parse();
        }
        break;
      } else if (token.second == "out") {
        ASTNode *out_node = new ASTNode("STATEMENT", "out");
        ASTNode *functionCall_node = new ASTNode("FUNCTIONCALL", "out");

        token = lexer_.getNextToken();

        if (token.first == TokenType::ROUND_PAREN) {
          std::tuple<bool, std::string, std::string> tokenInfo =
              isNextTokenLiteralOrIdentifier();

          bool isLiteralOrIdentifier = std::get<0>(tokenInfo);
          std::string tokenType = std::get<1>(tokenInfo);
          std::string tokenValue = std::get<2>(tokenInfo);

          if (isLiteralOrIdentifier) {
            ASTNode *literal_node = new ASTNode(tokenType, tokenValue);
            functionCall_node->add_child(literal_node);
            out_node->add_child(functionCall_node);

            token = lexer_.getNextToken();
            if (token.first == TokenType::ROUND_PAREN) {
              current_parent_->add_child(out_node);
              parse();
            } else {
              std::cerr << "Syntax error: Unexpected token '" << token.second
                        << "' Line: " << lexer_.getCurrentLineNumber()
                        << std::endl;
              return nullptr;
            }
          }
        } else {
          std::cerr << "Syntax error: Unexpected token '" << token.second
                    << "' Line: " << lexer_.getCurrentLineNumber() << std::endl;
          return nullptr;
        }
      } else {
        std::cerr << "Syntax error: Unexpected token '" << token.second
                  << "' Line: " << lexer_.getCurrentLineNumber() << std::endl;
        return nullptr;
      }
      break;
    case TokenType::IDENTIFIER:
      break;
    case TokenType::CURLY_PAREN:
      if (token.second == "{") {
        parse();
      } else if (token.second == "}") {
        popParentNode();
        parse();
      }
      break;
    case TokenType::INT:
      if (parseVarAssignment(TokenType::NUMERIC_LITERAL, "INT")) {
        parse();
      } else {
        std::cerr << "Syntax error: Unexpected token '" << token.second
                  << "' Line: " << lexer_.getCurrentLineNumber() << std::endl;
        return nullptr;
      }
      break;
    case TokenType::FLOAT:
      if (parseVarAssignment(TokenType::NUMERIC_LITERAL, "FLOAT")) {
        parse();
      } else {
        std::cerr << "Syntax error: Unexpected token '" << token.second
                  << "' Line: " << lexer_.getCurrentLineNumber() << std::endl;
        return nullptr;
      }
      break;
    case TokenType::STRING:
      if (parseVarAssignment(TokenType::STRING_LITERAL, "STRING")) {
        parse();
      } else {
        std::cerr << "Syntax error: Unexpected token '" << token.second
                  << "' Line: " << lexer_.getCurrentLineNumber() << std::endl;
        return nullptr;
      }
      break;
    case TokenType::CHAR:
      if (parseVarAssignment(TokenType::CHAR_LITERAL, "CHAR")) {
        parse();
      } else {
        std::cerr << "Syntax error: Unexpected token '" << token.second
                  << "' Line: " << lexer_.getCurrentLineNumber() << std::endl;
        return nullptr;
      }
      break;
    case TokenType::BOOL:
      if (parseVarAssignment(TokenType::BOOL_LITERAL, "BOOL")) {
        parse();
      } else {
        std::cerr << "Syntax error: Unexpected token '" << token.second
                  << "' Line: " << lexer_.getCurrentLineNumber() << std::endl;
        return nullptr;
      }
      break;
    case TokenType::PUNCTUATION:
      break;
    case TokenType::NONE:
      break;
    case TokenType::ERROR:
      std::cerr << "Syntax error: Unexpected token '" << token.second
                << "' Line: " << lexer_.getCurrentLineNumber() << std::endl;
      return nullptr;
    case TokenType::END_OF_FILE:
      break;
    default:
      std::cerr << "Syntax error: Unexpected token '" << token.second
                << "' Line: " << lexer_.getCurrentLineNumber() << std::endl;
      return nullptr;
    }
  } while (token.first != TokenType::END_OF_FILE);

  return root;
}

Condition Parser::parseCondition() {
  std::pair<TokenType, std::string> token;

  std::pair<TokenType, std::string> left;
  std::pair<TokenType, std::string> op;
  std::pair<TokenType, std::string> right;
  bool parsingPart1 = true;

  // Get the opening round parenthesis
  token = lexer_.getNextToken();

  if (token.first != TokenType::ROUND_PAREN) {
    std::cerr << "Syntax error: Expected '('" << std::endl;
    return {{TokenType::UNKNOWN, ""},
            {TokenType::UNKNOWN, ""},
            {TokenType::UNKNOWN, ""},
            true};
  }

  // Get the next token for the condition parsing
  token = lexer_.getNextToken();

  while (token.first != TokenType::ROUND_PAREN) {
    if (parsingPart1 && (token.first == TokenType::IDENTIFIER ||
                         token.first == TokenType::STRING_LITERAL ||
                         token.first == TokenType::NUMERIC_LITERAL ||
                         token.first == TokenType::CHAR_LITERAL ||
                         token.first == TokenType::BOOL_LITERAL)) {
      if (token.first == TokenType::IDENTIFIER &&
          std::find(uniqueNameList_.begin(), uniqueNameList_.end(),
                    token.second) != uniqueNameList_.end()) {
        left = token;
        parsingPart1 = false;
      } else if (token.first == TokenType::IDENTIFIER &&
                 std::find(uniqueNameList_.begin(), uniqueNameList_.end(),
                           token.second) == uniqueNameList_.end()) {
        std::cerr << "Variable " << token.second
                  << " does not exists! Line: " << lexer_.getCurrentLineNumber()
                  << std::endl;
        return {{TokenType::UNKNOWN, ""},
                {TokenType::UNKNOWN, ""},
                {TokenType::UNKNOWN, ""},
                true};
      } else {
        left = token;
        parsingPart1 = false;
      }
    } else if (!parsingPart1 && token.first == TokenType::RELATIONAL_OPERATOR) {
      op = token; // Capture the operator
    } else if (!parsingPart1 && (token.first == TokenType::IDENTIFIER ||
                                 token.first == TokenType::STRING_LITERAL ||
                                 token.first == TokenType::NUMERIC_LITERAL ||
                                 token.first == TokenType::CHAR_LITERAL ||
                                 token.first == TokenType::BOOL_LITERAL)) {
      right = token; // Capture the right side of the condition
    }

    token = lexer_.getNextToken();
  }

  // Check for completeness of condition
  if (left.first == TokenType::UNKNOWN || op.first == TokenType::UNKNOWN ||
      right.first == TokenType::UNKNOWN) {
    std::cerr << "Syntax error: Incomplete condition in 'if' statement! Line: "
              << lexer_.getCurrentLineNumber() << std::endl;
    return {{TokenType::UNKNOWN, ""},
            {TokenType::UNKNOWN, ""},
            {TokenType::UNKNOWN, ""},
            true};
  } else {
    return {left, op, right, false}; // Return the parsed condition
  }
}

ForLoopCondition Parser::parseForLoopCondition() {
  std::pair<TokenType, std::string> token;
  ForLoopCondition condition = {};

  token = lexer_.getNextToken();
  if (token.first != TokenType::ROUND_PAREN) {
    std::cerr << "Syntax error: Expected '(' in 'for' loop condition! Line: "
              << lexer_.getCurrentLineNumber() << std::endl;
    condition.error = true;
    return condition;
  }

  token = lexer_.getNextToken();
  if (token.first != TokenType::INT) {
    std::cerr << "Syntax error: Expected variable type in 'for' loop "
                 "condition! Line: "
              << lexer_.getCurrentLineNumber() << std::endl;
    condition.error = true;
    return condition;
  }
  condition.cInt = token;

  token = lexer_.getNextToken();
  if (token.first != TokenType::IDENTIFIER) {
    std::cerr
        << "Syntax error: Expected identifier in 'for' loop condition! Line: "
        << lexer_.getCurrentLineNumber() << std::endl;
    condition.error = true;
    return condition;
  }
  condition.i = token;

  token = lexer_.getNextToken();
  if (token.first != TokenType::ASSIGNMENT) {
    std::cerr
        << "Syntax error: Expected '=' in 'for' loop initialization! Line: "
        << lexer_.getCurrentLineNumber() << std::endl;
    condition.error = true;
    return condition;
  }

  token = lexer_.getNextToken();
  if (token.first != TokenType::NUMERIC_LITERAL) {
    std::cerr << "Syntax error: Expected numeric literal in 'for' loop "
                 "condition! Line: "
              << lexer_.getCurrentLineNumber() << std::endl;
    condition.error = true;
    return condition;
  }
  condition.nl = token;

  token = lexer_.getNextToken();
  if (token.first != TokenType::PUNCTUATION) {
    std::cerr << "Syntax error: Expected ';' in 'for' loop condition! Line: "
              << lexer_.getCurrentLineNumber() << std::endl;
    condition.error = true;
    return condition;
  }

  token = lexer_.getNextToken();
  if (token.first != TokenType::IDENTIFIER) {
    std::cerr
        << "Syntax error: Expected identifier in 'for' loop condition! Line: "
        << lexer_.getCurrentLineNumber() << std::endl;
    condition.error = true;
    return condition;
  }
  condition.i2 = token;

  token = lexer_.getNextToken();

  if (token.first != TokenType::RELATIONAL_OPERATOR) {
    std::cerr << "Syntax error: Expected relational operator in 'for' loop "
                 "condition! Line: "
              << lexer_.getCurrentLineNumber() << std::endl;
    condition.error = true;
    return condition;
  }

  if (token.first == TokenType::RELATIONAL_OPERATOR && token.second == "<") {
    condition.ro = token;
  } else {
    std::cerr << "Syntax error: Invalid relational operator in 'for' loop "
                 "condition! Line: "
              << lexer_.getCurrentLineNumber() << std::endl;
    condition.error = true;
    return condition;
  }

  token = lexer_.getNextToken();
  if (token.first != TokenType::IDENTIFIER &&
      token.first != TokenType::NUMERIC_LITERAL) {
    std::cerr << "Syntax error: Expected length variable or numeric literal in "
                 "'for' loop condition! Line: "
              << lexer_.getCurrentLineNumber() << std::endl;
    condition.error = true;
    return condition;
  }
  condition.len = token;

  token = lexer_.getNextToken();
  if (token.first != TokenType::PUNCTUATION) {
    std::cerr << "Syntax error: Expected ';' in 'for' loop condition! Line: "
              << lexer_.getCurrentLineNumber() << std::endl;
    condition.error = true;
    return condition;
  }

  token = lexer_.getNextToken();
  if (token.first != TokenType::IDENTIFIER) {
    std::cerr
        << "Syntax error: Expected identifier in 'for' loop condition! Line: "
        << lexer_.getCurrentLineNumber() << std::endl;
    condition.error = true;
    return condition;
  }
  condition.i3 = token;

  token = lexer_.getNextToken();
  if (token.first != TokenType::UNARY_ARITHMETIC_OPERATOR && token.second != "++" && token.second != "--") {
    std::cerr << "Syntax error: Expected increment/decrement operator in 'for' "
                 "loop condition! Line: "
              << lexer_.getCurrentLineNumber() << std::endl;
    condition.error = true;
    return condition;
  }
  condition.uao = token;

  token = lexer_.getNextToken();
  if (token.first != TokenType::ROUND_PAREN && token.second != ")") {
    std::cerr << "Syntax error: Expected ')' in 'for' loop condition! Line: "
              << lexer_.getCurrentLineNumber() << std::endl;
    condition.error = true;
    return condition;
  }

  return condition;
}

bool Parser::parseVarAssignment(TokenType varLiteralType, std::string varType) {
  std::pair<TokenType, std::string> token;

  ASTNode *varDeclaration_node = new ASTNode("VAR_DECLARATION", "");
  ASTNode *identifier_node = new ASTNode("IDENTIFIER", "");
  ASTNode *type_node = new ASTNode("VAR_TYPE", varType);
  ASTNode *assignment_node = new ASTNode("ASSIGNMENT", "");
  ASTNode *literal_node = new ASTNode(tokenTypeToString(varLiteralType), "");

  token = lexer_.getNextToken();
  if (token.first != TokenType::IDENTIFIER) {
    std::cerr << "Syntax error: Unexpected token '" << token.second
              << "' Line: " << lexer_.getCurrentLineNumber() << std::endl;
    return false;
  } else {
    if (std::find(uniqueNameList_.begin(), uniqueNameList_.end(),
                  token.second) == uniqueNameList_.end()) {
      uniqueNameList_.push_back(token.second);
      identifier_node->set_value(token.second);
    } else {
      std::cerr << "Syntax error: Variable already exists! Line: "
                << lexer_.getCurrentLineNumber() << std::endl;
      return false;
    }
    identifier_node->set_value(token.second);
  }

  token = lexer_.getNextToken();
  if (token.first != TokenType::ASSIGNMENT) {
    std::cerr << "Syntax error: Unexpected token '" << token.second
              << "' Line: " << lexer_.getCurrentLineNumber() << std::endl;
    return false;
  }

  token = lexer_.getNextToken();
  if (token.first != varLiteralType) {
    std::cerr << "Syntax error: Unexpected token '" << token.second
              << "' Line: " << lexer_.getCurrentLineNumber() << std::endl;
    return false;
  } else {
    literal_node->set_value(token.second);
  }

  assignment_node->add_child(literal_node);
  varDeclaration_node->add_child(type_node);
  varDeclaration_node->add_child(identifier_node);
  varDeclaration_node->add_child(assignment_node);

  current_parent_->add_child(varDeclaration_node);

  return true;
}

std::tuple<bool, std::string, std::string>
Parser::isNextTokenLiteralOrIdentifier() {
  std::pair<TokenType, std::string> token = lexer_.getNextToken();
  TokenType tokenType = token.first;
  bool isLiteralOrIdentifier = (tokenType == TokenType::STRING_LITERAL ||
                                tokenType == TokenType::NUMERIC_LITERAL ||
                                tokenType == TokenType::CHAR_LITERAL ||
                                tokenType == TokenType::BOOL_LITERAL ||
                                tokenType == TokenType::IDENTIFIER);

  return std::make_tuple(isLiteralOrIdentifier, tokenTypeToString(tokenType),
                         token.second);
}

std::string Parser::tokenTypeToString(TokenType tokenType) {
  switch (tokenType) {
  case TokenType::KEYWORD:
    return "KEYWORD";
  case TokenType::IDENTIFIER:
    return "IDENTIFIER";
  case TokenType::STRING_LITERAL:
    return "STRING_LITERAL";
  case TokenType::CHAR_LITERAL:
    return "CHAR_LITERAL";
  case TokenType::NUMERIC_LITERAL:
    return "NUMERIC_LITERAL";
  case TokenType::BOOL_LITERAL:
    return "BOOL_LITERAL";
  case TokenType::PUNCTUATION:
    return "PUNCTUATION";
  case TokenType::ASSIGNMENT:
    return "ASSIGNMENT";
  case TokenType::ROUND_PAREN:
    return "ROUND_PAREN";
  case TokenType::CURLY_PAREN:
    return "CURLY_PAREN";
  case TokenType::SQUARE_PAREN:
    return "SQUARE_PAREN";
  case TokenType::RELATIONAL_OPERATOR:
    return "RELATIONAL_OPERATOR";
  case TokenType::COMMA:
    return "COMMA";
  case TokenType::MATH_OPERATOR:
    return "MATH_OPERATOR";
  case TokenType::INT:
    return "INT";
  case TokenType::FLOAT:
    return "FLOAT";
  case TokenType::CHAR:
    return "CHAR";
  case TokenType::STRING:
    return "STRING";
  case TokenType::BOOL:
    return "BOOL";
  case TokenType::UNARY_ARITHMETIC_OPERATOR:
    return "UNARY_ARITHMETIC_OPERATOR";
  case TokenType::END_OF_FILE:
    return "END_OF_FILE";
  case TokenType::NONE:
    return "NONE";
  case TokenType::ERROR:
    return "ERROR";
  default:
    return "UNKNOWN";
  }
}

void Parser::switchParentNode(ASTNode *new_parent) {
  current_parent_ = new_parent;
  scope_stack_.push_back(new_parent);
}

void Parser::popParentNode() {
  if (!scope_stack_.empty()) {
    scope_stack_.pop_back();
    if (!scope_stack_.empty()) {
      current_parent_ = scope_stack_.back();
    } else {
      current_parent_ = nullptr;
    }
  }
}

void printAST(ASTNode *node, int depth) {
  if (node == nullptr) {
    return;
  }

  for (int i = 0; i < depth; ++i) {
    std::cout << "  ";
  }

  std::cout << "Type: " << node->getType() << ", Value: " << node->getValue()
            << std::endl;

  const std::vector<ASTNode *> &children = node->getChildren();
  for (ASTNode *child : children) {
    printAST(child, depth + 1);
  }
}
