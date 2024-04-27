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
        void parseKEYWORDCode();
};

Parser::Parser(const std::string& filename) : lexer_(filename) {}

void Parser::parse() {
    std::pair<TokenType, std::string> token;

    do {
        token = lexer_.getNextToken();

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
                } else {
                    std::cerr << "Syntax error: Unexpected token '" << token.second << "'" << std::endl;
                    return;
                }
                // Add cases for other keywords
                break;
            case TokenType::IDENTIFIER:
                // Handle identifiers
                break;
            case TokenType::ERROR:
                // Throw syntax error
                std::cerr << "Syntax error: Unexpected token '" << token.second << "'" << std::endl;
                return;
            default:
                // Handle other token types if needed
                break;
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

        while (token.first == TokenType::ASSIGNMENT && _operator.size() < 2) {
            _operator += token.second;
            token = lexer_.getNextToken();
        }

        if (_part2.empty()) {
            _part2 = token.second;
        }
        
        token = lexer_.getNextToken();
    }

    // Check if all condition parts are parsed correctly
    if (_part1.empty() || _operator.empty() || _part2.empty()) {
        std::cerr << "Syntax error: Incomplete condition in 'if' statement" << std::endl;
        return true;
    } else {
        return false;
    }
}

int main() {
    Parser parser("test.vg");
    parser.parse();

    return 0;
}