#include <iostream>
#include <string>
#include <fstream>
#include <regex>
#include "lexer.cpp"

class Parser {
    public:
        Parser(const std::string& filename);
        void parse();
    private:
        Lexer lexer_;
        bool parseCondition();
        bool parseVarAssignment(TokenType varType);
        bool isNextTokenLiteralOrIdentifier();
};

Parser::Parser(const std::string& filename) : lexer_(filename) {}

void Parser::parse() {
    std::pair<TokenType, std::string> token;

    do {
        token = lexer_.getNextToken();

        std::cout << token.second << std::endl;

        switch (token.first) {
            case TokenType::KEYWORD:
                if (token.second == "if") {
                    if (parseCondition()) {
                        parse();
                    }
                } else if (token.second == "else") {
                    parse();
                } else if (token.second == "while") {
                    if (parseCondition()) {
                        parse();
                    }
                } else if (token.second == "for") {
                    if (parseCondition()) {
                        parse();
                    }
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
            case TokenType::ERROR:
                // Throw syntax error
                std::cerr << "Syntax error: Unexpected token '" << token.second << "' Line: " << lexer_.getCurrentLineNumber() << std::endl;
                return;
            default:
                std::cerr << "Syntax error: Unexpected token '" << token.second << "' Line: " << lexer_.getCurrentLineNumber() << std::endl;
                return;
        }
    } while (token.first != TokenType::END_OF_FILE);
}

bool Parser::parseCondition() {
    std::pair<TokenType, std::string> token;

    // Initialize condition variables
    std::string _part1;
    std::string _operator;
    std::string _part2;

    // Get next token
    token = lexer_.getNextToken();

    // Check if token is ROUND_PAREN
    if (token.first != TokenType::ROUND_PAREN) {
        std::cerr << "Syntax error: Expected '('" << std::endl;
        return false;
    }

    // Get next token
    token = lexer_.getNextToken();

    // Parse condition
    while (token.first != TokenType::CURLY_PAREN) {
        if (_part1.empty()) {
            _part1 = token.second;
        }

        if (token.first == TokenType::RELATIONAL_OPERATOR && _operator.empty()) {
            _operator = token.second;
        }

        if (_part2.empty()) {
            _part2 = token.second;
        }
        
        token = lexer_.getNextToken();
    }

    // Check if all condition parts are parsed correctly
    if (_part1.empty() || _operator.empty() || _part2.empty()) {
        std::cerr << "Syntax error: Incomplete condition in 'if' statement! Line: " << lexer_.getCurrentLineNumber() << std::endl;
        return true;
    } else {
        return false;
    }
}

bool Parser::parseVarAssignment(TokenType varType) {
    std::pair<TokenType, std::string> token;

    // std::cout << token.second << std::endl;

    token = lexer_.getNextToken();
    if (token.first != TokenType::IDENTIFIER) {
        std::cerr << "Syntax error: Unexpected token '" << token.second << "' Line: " << lexer_.getCurrentLineNumber() << std::endl;
        return false;
    }

    // std::cout << token.second << std::endl;

    token = lexer_.getNextToken();
    if (token.first != TokenType::ASSIGNMENT) {
        std::cerr << "Syntax error: Unexpected token '" << token.second << "' Line: " << lexer_.getCurrentLineNumber() << std::endl;
        return false;
    }

    // std::cout << token.second << std::endl;

    token = lexer_.getNextToken();
    if (token.first != varType) {
        std::cerr << "Syntax error: Unexpected token '" << token.second << "' Line: " << lexer_.getCurrentLineNumber() << std::endl;
        return false;
    }

    // std::cout << token.second << std::endl;

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
    Parser parser("test.vg");
    parser.parse();

    return 0;
}