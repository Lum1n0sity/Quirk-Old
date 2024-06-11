#ifndef LEXER_H
#define LEXER_H

#include <iostream>
#include <string>
#include <fstream>
#include <regex>
#include <utility>

enum TokenType {
    KEYWORD,
    IDENTIFIER,
    STRING_LITERAL,
    CHAR_LITERAL,
    NUMERIC_LITERAL,
    BOOL_LITERAL,
    PUNCTUATION,
    ASSIGNMENT,
    ROUND_PAREN,
    CURLY_PAREN,
    SQUARE_PAREN,
    RELATIONAL_OPERATOR,
    COMMA,
    MATH_OPERATOR,
    INT,
    FLOAT,
    CHAR,
    STRING,
    BOOL,
    UNARY_ARITHMETIC_OPERATOR,
    END_OF_FILE,
    NONE,
    ERROR,
};

class Lexer {
public:
    Lexer(const std::string& filename);
    std::pair<TokenType, std::string> getNextToken();
    bool isMathOperator(char c);
    std::string getCurrentLineNumber() const;
    ~Lexer();

private:
    std::string filename_;
    std::ifstream file_;
    int currentPos_;
    int lineNumber_;
    const std::string MATH_OPERATORS;
    const std::regex KEYWORD_REGEX;
    const std::regex IDENTIFIER_REGEX;
    const std::regex NUMERIC_LITERAL_REGEX;
    const std::regex STRING_LITERAL_REGEX;
    const std::regex CHAR_LITERAL_REGEX;
    const std::regex BOOL_LITERAL_REGEX;
    const std::regex INT_REGEX;
    const std::regex FLOAT_REGEX;
    const std::regex STRING_REGEX;
    const std::regex CHAR_REGEX;
    const std::regex BOOL_REGEX;
    const std::regex RELATIONAL_OPERATOR_REGEX;
    const std::regex UNARY_ARITHMETIC_OPERATOR_REGEX;
};

void compileFile(const std::string& filename);

#endif