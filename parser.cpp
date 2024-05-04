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

    private:
        std::string type;
        std::string value;
        std::vector<ASTNode*> children;
};

class Parser {
    public:
        Parser(const std::string& filename);

        ASTNode* parse();
        Condition parseCondition();
    private:
        Lexer lexer_;
        bool parseVarAssignment(TokenType varType);
        bool isNextTokenLiteralOrIdentifier();
};

Parser::Parser(const std::string& filename) : lexer_(filename) {}

Parser parser;

// TODO: Add Node parent switching

ASTNode* Parser::parse() {
    ASTNode* root = new ASTNode("Program");
    std::pair<TokenType, std::string> token;

    do {
        token = lexer_.getNextToken();

        switch (token.first) {
            case TokenType::KEYWORD:
                if (token.second == "if") {
                    ASTNode* if_decl = new ASTNode("IF", "if");

                    Condition condition = parser.parseCondition();

                    if (condition.error) {
                        std::cerr << "Error parsing condition" << std::endl;
                        break;
                    } else {
                        std::string combinedCondition = condition.part1 + condition.op + condition.part2;
                        ASTNode* conditionNode = new ASTNode("Condition", combinedCondition);
                        root->add_child(if_decl);
                        parse();
                    }
                } else if (token.second == "else") {
                    parse();
                } else if (token.second == "while") {

                } else if (token.second == "for") {

                } else if (token.second == "out") {
                    token = lexer_.getNextToken();
                    if (token.first == TokenType::ROUND_PAREN) {
                        if (isNextTokenLiteralOrIdentifier()) {
                            token = lexer_.getNextToken();
                            if (token.first == TokenType::ROUND_PAREN) {
                                parse();
                            }
                        }
                    } else {
                        std::cerr << "Syntax error: Unexpected token '" << token.second << "' Line: " << lexer_.getCurrentLineNumber() << std::endl;
                        return;                        
                    }
                } else {
                    std::cerr << "Syntax error: Unexpected token '" << token.second << "' Line: " << lexer_.getCurrentLineNumber() << std::endl;
                    return;
                }
                // Add cases for other keywords
                break;
            case TokenType::IDENTIFIER:
                // Handle identifiers
                break;
            case TokenType::CURLY_PAREN:
                break;
            case TokenType::INT:
                if (parseVarAssignment(TokenType::NUMERIC_LITERAL)) {
                    parse();
                } else {
                    std::cerr << "Syntax error: Unexpected token '" << token.second << "' Line: " << lexer_.getCurrentLineNumber() << std::endl;
                    return;
                }
                break;
            case TokenType::FLOAT:
                if (parseVarAssignment(TokenType::NUMERIC_LITERAL)) {
                    parse();
                } else {
                    std::cerr << "Syntax error: Unexpected token '" << token.second << "' Line: " << lexer_.getCurrentLineNumber() << std::endl;
                    return;
                }
                break;
            case TokenType::STRING:
                if (parseVarAssignment(TokenType::STRING_LITERAL)) {
                    parse();
                } else {
                    std::cerr << "Syntax error: Unexpected token '" << token.second << "' Line: " << lexer_.getCurrentLineNumber() << std::endl;
                    return;                    
                }
                break; 
            case TokenType::CHAR:
                if (parseVarAssignment(TokenType::CHAR_LITERAL)) {
                    parse();
                } else {
                    std::cerr << "Syntax error: Unexpected token '" << token.second << "' Line: " << lexer_.getCurrentLineNumber() << std::endl;
                    return;
                }
                break;
            case TokenType::BOOL:
                if (parseVarAssignment(TokenType::BOOL_LITERAL)) {
                    parse();
                } else {
                    std::cerr << "Syntax error: Unexpected token '" << token.second << "' Line: " << lexer_.getCurrentLineNumber() << std::endl;
                    return;
                }
                break;
            case TokenType::PUNCTUATION:
                break;
            case TokenType::NONE:
                break;
            case TokenType::ERROR:
                // Throw syntax error
                std::cerr << "Syntax error: Unexpected token '" << token.second << "' Line: " << lexer_.getCurrentLineNumber() << std::endl;
                return;
            case TokenType::END_OF_FILE:
                break;
            default:
                std::cerr << "Syntax error: Unexpected token '" << token.second << "' Line: " << lexer_.getCurrentLineNumber() << std::endl;
                return;
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
        if (part1.empty()) {
            part1 = token.second;
        }

        if (token.first == TokenType::RELATIONAL_OPERATOR && op.empty()) {
            op = token.second;
        }

        if (part2.empty()) {
            part2 = token.second;
        }
        
        token = lexer_.getNextToken();
    }

    // Check if all condition parts are parsed correctly
    if (part1.empty() || op.empty() || part2.empty()) {
        std::cerr << "Syntax error: Incomplete condition in 'if' statement! Line: " << lexer_.getCurrentLineNumber() << std::endl;
        return { "", "", "", true }; // Return empty Condition struct with error flag set
    } else {
        return { part1, op, part2, false }; // Return Condition struct with parsed parts and no error
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

bool Parser::isNextTokenLiteralOrIdentifier() {
    std::pair<TokenType, std::string> token = lexer_.getNextToken();
    TokenType tokenType = token.first;
    return (tokenType == TokenType::STRING_LITERAL || tokenType == TokenType::NUMERIC_LITERAL || 
            tokenType == TokenType::CHAR_LITERAL || tokenType == TokenType::BOOL_LITERAL || 
            tokenType == TokenType::IDENTIFIER);
}

int main() {
    Parser parser("test.qk");

    parser.parse();

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