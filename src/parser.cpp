#include "parser.h"
#include "lexer.h"

#include "lexer.cpp"

Parser::Parser(const std::string& filename) : lexer_(filename), current_parent_(nullptr) {}  

ASTNode* root = new ASTNode("Program");

void Parser::Initalize() {
    current_parent_ = root;
    scope_stack_.push_back(root);
}

ASTNode* Parser::parse() {
    std::pair<TokenType, std::string> token;

    do {
        token = lexer_.getNextToken();

        switch (token.first) {
            case TokenType::KEYWORD:
                if (token.second == "if") {
                    ASTNode* if_node = new ASTNode("STATEMENT", "if");

                    Condition condition = parseCondition();

                    if (condition.error) {
                        ASTNode* error_node = new ASTNode("ERROR", "error");

                        root->add_child(error_node);
                        return nullptr;
                    } else {
                        std::string combinedCondition = condition.part1 + condition.op + condition.part2;

                        ASTNode* condition_node = new ASTNode("CONDITION", combinedCondition);
                        ASTNode* codeBlock_node = new ASTNode("CODE_BLOCK", "");

                        if_node->add_child(condition_node);
                        if_node->add_child(codeBlock_node);

                        current_parent_->add_child(if_node);

                        switchParentNode(codeBlock_node);
                    }
                } else if (token.second == "else") {
                    token = lexer_.getNextToken();
                    if (token.first == TokenType::CURLY_PAREN) {
                        ASTNode* else_node = new ASTNode("STATEMENT", "else");
                        ASTNode* codeBlock_node = new ASTNode("CODE_BLOCK", "");

                        else_node->add_child(codeBlock_node);
                        current_parent_->add_child(else_node);

                        switchParentNode(codeBlock_node);

                        parse();
                    }
                } else if (token.second == "while") {
                    ASTNode* while_node = new ASTNode("STATEMENT", "while");

                    Condition condition = parseCondition();

                    if (condition.error) {
                        ASTNode* error_node = new ASTNode("ERROR", "error");

                        root->add_child(error_node);
                        return nullptr;
                    } else {
                        std::string combinedCondition = condition.part1 + condition.op + condition.part2;

                        ASTNode* condition_node = new ASTNode("CONDITION", combinedCondition);
                        ASTNode* codeBlock_node = new ASTNode("CODE_BLOCK", "");

                        while_node->add_child(condition_node);
                        while_node->add_child(codeBlock_node);

                        current_parent_->add_child(while_node);

                        switchParentNode(codeBlock_node);

                        parse();
                    }
                } else if (token.second == "for") {
                    ASTNode* for_node = new ASTNode("STATEMENT", "for");

                    ForLoopCondition condition = parseForLoopCondition();

                    if (condition.error) {
                        std::cerr << "" << std::endl;
                        ASTNode* error_node = new ASTNode("ERROR", "error");

                        root->add_child(error_node);
                        return nullptr;
                    } else {
                        std::string combinedCondition = condition.initialization + condition.condition + condition.update;

                        ASTNode* condition_node = new ASTNode("CONDITON", combinedCondition);
                        ASTNode* codeBlock_node = new ASTNode("CODE_BLOCK", "");

                        for_node->add_child(condition_node);
                        for_node->add_child(codeBlock_node);

                        current_parent_->add_child(for_node);

                        switchParentNode(for_node);

                        parse();
                    }
                    break;
                } else if (token.second == "out") {
                    ASTNode* out_node = new ASTNode("STATEMENT", "out");
                    ASTNode* functionCall_node = new ASTNode("FUNCTIONCALL", "out");

                    token = lexer_.getNextToken();

                    if (token.first == TokenType::ROUND_PAREN) {
                        std::pair<bool, std::string> tokenInfo = isNextTokenLiteralOrIdentifier();

                        bool isLiteralOrIdentifier = tokenInfo.first;
                        std::string tokenValue = tokenInfo.second;

                        if (isLiteralOrIdentifier) {
                            ASTNode* literal_node = new ASTNode("LITERAL/IDENTIFIER", tokenValue);

                            functionCall_node->add_child(literal_node);
                            out_node->add_child(functionCall_node);

                            token = lexer_.getNextToken();
                            if (token.first == TokenType::ROUND_PAREN) {
                                current_parent_->add_child(out_node);
                                parse();
                            } else {
                                std::cerr << "Syntax error: Unexpected token '" << token.second << "' Line: " << lexer_.getCurrentLineNumber() << std::endl;
                                return nullptr;   
                            }
                        }
                    } else {
                        std::cerr << "Syntax error: Unexpected token '" << token.second << "' Line: " << lexer_.getCurrentLineNumber() << std::endl;
                        return nullptr;                        
                    }
                } else {
                    std::cerr << "Syntax error: Unexpected token '" << token.second << "' Line: " << lexer_.getCurrentLineNumber() << std::endl;
                    return nullptr;
                }
                break;
            case TokenType::IDENTIFIER:
                // Handle identifiers
                break;
            case TokenType::CURLY_PAREN:
                if (token.second == "{") {
                    ASTNode* codeBlock_node = new ASTNode("CODE_BLOCK", "");
                    current_parent_->add_child(codeBlock_node);
                    switchParentNode(codeBlock_node);
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
                    std::cerr << "Syntax error: Unexpected token '" << token.second << "' Line: " << lexer_.getCurrentLineNumber() << std::endl;
                    return nullptr;
                }
                break;
            case TokenType::FLOAT:
                if (parseVarAssignment(TokenType::NUMERIC_LITERAL, "FLOAT")) {
                    parse();
                } else {
                    std::cerr << "Syntax error: Unexpected token '" << token.second << "' Line: " << lexer_.getCurrentLineNumber() << std::endl;
                    return nullptr;
                }
                break;
            case TokenType::STRING:
                if (parseVarAssignment(TokenType::STRING_LITERAL, "STRING")) {
                    parse();
                } else {
                    std::cerr << "Syntax error: Unexpected token '" << token.second << "' Line: " << lexer_.getCurrentLineNumber() << std::endl;
                    return nullptr;
                }
                break; 
            case TokenType::CHAR:
                if (parseVarAssignment(TokenType::CHAR_LITERAL, "CHAR")) {
                    parse();
                } else {
                    std::cerr << "Syntax error: Unexpected token '" << token.second << "' Line: " << lexer_.getCurrentLineNumber() << std::endl;
                    return nullptr;
                }
                break;
            case TokenType::BOOL:
                if (parseVarAssignment(TokenType::BOOL_LITERAL, "BOOL")) {
                    parse();
                } else {
                    std::cerr << "Syntax error: Unexpected token '" << token.second << "' Line: " << lexer_.getCurrentLineNumber() << std::endl;
                    return nullptr;
                }
                break;
            case TokenType::PUNCTUATION:
                break;
            case TokenType::NONE:
                break;
            case TokenType::ERROR:
                // Throw syntax error
                std::cerr << "Syntax error: Unexpected token '" << token.second << "' Line: " << lexer_.getCurrentLineNumber() << std::endl;
                return nullptr;
            case TokenType::END_OF_FILE:
                break;
            default:
                std::cerr << "Syntax error: Unexpected token '" << token.second << "' Line: " << lexer_.getCurrentLineNumber() << std::endl;
                return nullptr;
        }
    } while (token.first != TokenType::END_OF_FILE);

    return root;
}

Condition Parser::parseCondition() {
    std::pair<TokenType, std::string> token;

    std::string part1;
    std::string op;
    std::string part2;
    bool parsingPart1 = true;

    token = lexer_.getNextToken();

    if (token.first != TokenType::ROUND_PAREN) {
        std::cerr << "Syntax error: Expected '('" << std::endl;
        return { "", "", "", true };
    }

    token = lexer_.getNextToken();

    while (token.first != TokenType::CURLY_PAREN) {
        if (parsingPart1 && (token.first == TokenType::IDENTIFIER || token.first == TokenType::STRING_LITERAL || token.first == TokenType::NUMERIC_LITERAL || token.first == TokenType::CHAR_LITERAL || token.first == TokenType::BOOL_LITERAL)) {
            part1 = token.second;
            parsingPart1 = false;
        } else if (!parsingPart1 && token.first == TokenType::RELATIONAL_OPERATOR && op.empty()) {
            op = token.second;
        } else if (!parsingPart1 && (token.first == TokenType::IDENTIFIER || token.first == TokenType::STRING_LITERAL || token.first == TokenType::NUMERIC_LITERAL || token.first == TokenType::CHAR_LITERAL || token.first == TokenType::BOOL_LITERAL)) {
            part2 = token.second;
        }
        
        token = lexer_.getNextToken();
    }

    if (part1.empty() || op.empty() || part2.empty()) {
        std::cerr << "Syntax error: Incomplete condition in 'if' statement! Line: " << lexer_.getCurrentLineNumber() << std::endl;
        return { "", "", "", true };
    } else {
        return { part1, op, part2, false };
    }
}

ForLoopCondition Parser::parseForLoopCondition() {
    std::pair<TokenType, std::string> token;

    std::string initialization;
    std::string condition;
    std::string update;

    token = lexer_.getNextToken();

    if (token.first != TokenType::ROUND_PAREN) {
        std::cerr << "Syntax error: Expected '('" << std::endl;
        return { "", "", "", true }; // Return empty ForLoopCondition struct with error flag set
    }

    token = lexer_.getNextToken();

    while (token.first != TokenType::PUNCTUATION) {
        initialization += token.second;
        token = lexer_.getNextToken();
    }

    initialization += ';';

    token = lexer_.getNextToken();

    while (token.first != TokenType::PUNCTUATION) {
        condition += token.second;
        token = lexer_.getNextToken();
    }

    condition += ';';
    
    token = lexer_.getNextToken();

    while (token.first != TokenType::ROUND_PAREN) {
        update += token.second;
        token = lexer_.getNextToken();
    }

    if (initialization.empty() || condition.empty() || update.empty()) {
        std::cerr << "Syntax error: Incomplete condition in 'for' loop! Line: " << lexer_.getCurrentLineNumber() << std::endl;
        return { "", "", "", true };
    } else {
        return { initialization, condition, update, false };
    }
}

bool Parser::parseVarAssignment(TokenType varLiteralType, std::string varType) {
    std::pair<TokenType, std::string> token;

    ASTNode* varDeclaration_node = new ASTNode("VAR_DECLARATION", "");
    ASTNode* identifier_node = new ASTNode("IDENTIFIER", "");
    ASTNode* type_node = new ASTNode("VAR_TYPE", varType);
    ASTNode* assignment_node = new ASTNode("ASSIGNMENT", "");
    ASTNode* literal_node = new ASTNode("LITERAL", "");

    token = lexer_.getNextToken();
    if (token.first != TokenType::IDENTIFIER) {
        std::cerr << "Syntax error: Unexpected token '" << token.second << "' Line: " << lexer_.getCurrentLineNumber() << std::endl;
        return false;
    } else {
        identifier_node->set_value(token.second);
    }

    token = lexer_.getNextToken();
    if (token.first != TokenType::ASSIGNMENT) {
        std::cerr << "Syntax error: Unexpected token '" << token.second << "' Line: " << lexer_.getCurrentLineNumber() << std::endl;
        return false;
    }

    token = lexer_.getNextToken();
    if (token.first != varLiteralType) {
        std::cerr << "Syntax error: Unexpected token '" << token.second << "' Line: " << lexer_.getCurrentLineNumber() << std::endl;
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

std::pair<bool, std::string> Parser::isNextTokenLiteralOrIdentifier() {
    std::pair<TokenType, std::string> token = lexer_.getNextToken();
    TokenType tokenType = token.first;
    bool isLiteralOrIdentifier (tokenType == TokenType::STRING_LITERAL || tokenType == TokenType::NUMERIC_LITERAL || 
                                tokenType == TokenType::CHAR_LITERAL || tokenType == TokenType::BOOL_LITERAL || 
                                tokenType == TokenType::IDENTIFIER);

    return std::make_pair(isLiteralOrIdentifier, token.second);
}

void Parser::switchParentNode(ASTNode* new_parent) {
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

void printAST(ASTNode* node, int depth) {
    if (node == nullptr) {
        return;
    }

    for (int i = 0; i < depth; ++i) {
        std::cout << "  ";
    }
 
    std::cout << "Type: " << node->getType() << ", Value: " << node->getValue() << std::endl;

    const std::vector<ASTNode*>& children = node->getChildren();
    for (ASTNode* child : children) {
       printAST(child, depth + 1);
    }
}