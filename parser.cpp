#include <iostream>
#include <string>
#include <fstream>
#include <regex>
#include <optional>
#include "lexer.cpp"

struct Condition {
    std::string part1;
    std::string op;
    std::string part2;
    bool error;
};

class ASTNode {
public:
    ASTNode(std::string type, std::string value = "") : type(type), value(value) {}

    void add_child(ASTNode* node) {
        children.push_back(node);
    }

    void set_parent(ASTNode* parent) {
        parent_ = parent;
    }

    ASTNode* get_parent() const {
        return parent_;
    }

    std::string getType() const {
        return type;
    }

    std::string getValue() const {
        return value;
    }

    const std::vector<ASTNode*>& getChildren() const {
        return children;
    }

private:
    std::string type;
    std::string value;
    std::vector<ASTNode*> children;
    ASTNode* parent_;
};

class Parser {
public:
    Parser(const std::string& filename);

    ASTNode* parse();
    Condition parseCondition();
private:
    Lexer lexer_;
    ASTNode* current_parent_;
    std::vector<ASTNode*> scope_stack_;

    bool parseVarAssignment(TokenType varType);
    std::pair<bool, std::string> isNextTokenLiteralOrIdentifier();
    void switchParentNode(ASTNode* new_parent);
    void popParentNode();
};

Parser::Parser(const std::string& filename) : lexer_(filename), current_parent_(nullptr) {}
ASTNode* root = new ASTNode("Program");

ASTNode* Parser::parse() {
    current_parent_ = root;
    scope_stack_.push_back(root);
    
    

    std::pair<TokenType, std::string> token;

    // TODO fix parent pop back

    do {
        token = lexer_.getNextToken();

        switch (token.first) {
            case TokenType::KEYWORD:
                if (token.second == "if") {
                    ASTNode* if_node = new ASTNode("STATEMENT", "if");

                    Condition condition = parseCondition();

                    if (condition.error) {
                        std::cerr << "Error parsing condition" << std::endl;
                        break;
                    } else {
                        std::string combinedCondition = condition.part1 + condition.op + condition.part2;

                        ASTNode* conditionNode = new ASTNode("CONDITION", combinedCondition);
                        ASTNode* codeBlock_node = new ASTNode("CODE_BLOCK", "");

                        if_node->add_child(conditionNode);
                        if_node->add_child(codeBlock_node);

                        current_parent_->add_child(if_node);

                        switchParentNode(codeBlock_node);
                    }
                } else if (token.second == "else") {
                    token = lexer_.getNextToken();
                    if (token.first == TokenType::CURLY_PAREN) {
                        ASTNode* codeBlock_node = new ASTNode("CODE_BLOCK", "");

                        current_parent_->add_child(codeBlock_node);

                        switchParentNode(codeBlock_node);

                        parse();
                    }
                } else if (token.second == "while") {
                    ASTNode* while_node = new ASTNode("STATEMENT", "while");

                    Condition condition = parseCondition();

                    if (condition.error) {
                        std::cerr << "Error parsing condition" << std::endl;
                        break;
                    } else {
                        std::string combinedCondition = condition.part1 + condition.op + condition.part2;

                        ASTNode* conditionNode = new ASTNode("CONDITION", combinedCondition);
                        ASTNode* codeBlock_node = new ASTNode("CODE_BLOCK", "");

                        while_node->add_child(conditionNode);
                        while_node->add_child(codeBlock_node);

                        current_parent_->add_child(while_node);

                        switchParentNode(codeBlock_node);
                    }
                } else if (token.second == "for") {
                    // Handle 'for' keyword
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
                            current_parent_->add_child(out_node);

                            token = lexer_.getNextToken();
                            if (token.first == TokenType::ROUND_PAREN) {
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
                if (parseVarAssignment(TokenType::NUMERIC_LITERAL)) {
                    parse();
                } else {
                    std::cerr << "Syntax error: Unexpected token '" << token.second << "' Line: " << lexer_.getCurrentLineNumber() << std::endl;
                    return nullptr;
                }
                break;
            case TokenType::FLOAT:
                if (parseVarAssignment(TokenType::NUMERIC_LITERAL)) {
                    parse();
                } else {
                    std::cerr << "Syntax error: Unexpected token '" << token.second << "' Line: " << lexer_.getCurrentLineNumber() << std::endl;
                    return nullptr;
                }
                break;
            case TokenType::STRING:
                if (parseVarAssignment(TokenType::STRING_LITERAL)) {
                    parse();
                } else {
                    std::cerr << "Syntax error: Unexpected token '" << token.second << "' Line: " << lexer_.getCurrentLineNumber() << std::endl;
                    return nullptr;
                }
                break; 
            case TokenType::CHAR:
                if (parseVarAssignment(TokenType::CHAR_LITERAL)) {
                    parse();
                } else {
                    std::cerr << "Syntax error: Unexpected token '" << token.second << "' Line: " << lexer_.getCurrentLineNumber() << std::endl;
                    return nullptr;
                }
                break;
            case TokenType::BOOL:
                if (parseVarAssignment(TokenType::BOOL_LITERAL)) {
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

    // Initialize condition variables
    std::string part1;
    std::string op;
    std::string part2;
    bool parsingPart1 = true; // Flag to indicate if currently parsing part1

    // Get next token
    token = lexer_.getNextToken();

    // Check if token is ROUND_PAREN
    if (token.first != TokenType::ROUND_PAREN) {
        std::cerr << "Syntax error: Expected '('" << std::endl;
        return { "", "", "", true }; // Return empty Condition struct with error flag set
    }

    // Get next token
    token = lexer_.getNextToken();

    // Parse condition
    while (token.first != TokenType::CURLY_PAREN) {
        if (parsingPart1 && (token.first == TokenType::IDENTIFIER || token.first == TokenType::STRING_LITERAL || token.first == TokenType::NUMERIC_LITERAL || token.first == TokenType::CHAR_LITERAL || token.first == TokenType::BOOL_LITERAL)) {
            part1 = token.second;
            parsingPart1 = false; // Move to parsing part2
        } else if (!parsingPart1 && token.first == TokenType::RELATIONAL_OPERATOR && op.empty()) {
            op = token.second;
        } else if (!parsingPart1 && (token.first == TokenType::IDENTIFIER || token.first == TokenType::STRING_LITERAL || token.first == TokenType::NUMERIC_LITERAL || token.first == TokenType::CHAR_LITERAL || token.first == TokenType::BOOL_LITERAL)) {
            part2 = token.second;
        }
        
        token = lexer_.getNextToken();
    }

    // Check if all condition parts are parsed correctly
    if (part1.empty() || op.empty() || part2.empty()) {
        std::cerr << "Syntax error: Incomplete condition in 'if' statement! Line: " << lexer_.getCurrentLineNumber() << std::endl;
        return { "", "", "", true };
    } else {
        return { part1, op, part2, false };
    }
}

bool Parser::parseVarAssignment(TokenType varType) {
    std::pair<TokenType, std::string> token;

    token = lexer_.getNextToken();
    if (token.first != TokenType::IDENTIFIER) {
        std::cerr << "Syntax error: Unexpected token '" << token.second << "' Line: " << lexer_.getCurrentLineNumber() << std::endl;
        return false;
    }

    token = lexer_.getNextToken();
    if (token.first != TokenType::ASSIGNMENT) {
        std::cerr << "Syntax error: Unexpected token '" << token.second << "' Line: " << lexer_.getCurrentLineNumber() << std::endl;
        return false;
    }

    token = lexer_.getNextToken();
    if (token.first != varType) {
        std::cerr << "Syntax error: Unexpected token '" << token.second << "' Line: " << lexer_.getCurrentLineNumber() << std::endl;
        return false;
    }

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

void printAST(ASTNode* node, int depth = 0) {
    if (node == nullptr) {
        return;
    }

    // Print indentation based on depth
    for (int i = 0; i < depth; ++i) {
        std::cout << "  ";
    }

    // Print node type and value
    std::cout << "Type: " << node->getType() << ", Value: " << node->getValue() << std::endl;

    const std::vector<ASTNode*>& children = node->getChildren();
    for (ASTNode* child : children) {
       printAST(child, depth + 1);
    }
}

int main() {
    Parser parser("test.qk");

    ASTNode* root = parser.parse();

    printAST(root);

    return 0;
}

/*
    ASTNode* root = new ASTNode("Program");
    
    ASTNode* func_decl = new ASTNode("FunctionDeclaration", "my_function");
    
    ASTNode* param_list = new ASTNode("ParameterList");
    
    ASTNode* param1 = new ASTNode("Parameter", "param1");
    ASTNode* param2 = new ASTNode("Parameter", "param2");

    param_list->add_child(param1);
    param_list->add_child(param2);

    func_decl->add_child(param_list);

    root->add_child(func_decl);
*/