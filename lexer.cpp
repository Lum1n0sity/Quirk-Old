#include <iostream>
#include <string>
#include <fstream>
#include <regex>

enum TokenType {
	KEYWORD,
	IDENTIFIER,
	STRING_LITERAL,
	NUMERIC_LITERAL,
	PUNCTUATION,
	ASSIGNMENT,
	ROUND_PAREN,
	CURLY_PAREN,
	SQUARE_PAREN,
	COMMA,
	MATH_OPERATOR,
	INT,
	FLOAT,
	CHAR,
	STRING,
	BOOL,
	END_OF_FILE,
	UNKNOWN,
	NONE,
	ERROR,
};

class Lexer {
	public:
		Lexer(const std::string& filename) : filename_(filename), currentPos_(0) {
			file_.open(filename_, std::ios::in);
			if(!file_.is_open()) {
				std::cerr << "Error: Could not open file " << filename_ << std::endl;
			}
		}

		std::pair<TokenType, std::string> getNextToken() {
		    std::string tokenValue;

		    char c;

			while (file_.get(c) && std::isspace(c)) {}

		    if (file_.eof()) {
		        return std::make_pair(TokenType::END_OF_FILE, "");
		    }

			if (c == '"') {
				tokenValue += c;

				while (file_.get(c) && c != '"') {
					tokenValue += c;
				}
				tokenValue += c;
				return std::make_pair(TokenType::STRING_LITERAL, tokenValue);
			} else if (std::isdigit(c)) {
				tokenValue += c;
				while (file_.get(c) && std::isdigit(c)) {
					tokenValue += c;
				}

				if (c == '.') {
					tokenValue += c;
					while (file_.get(c) && std::isdigit(c)) {
						tokenValue += c;
					}
					
					return std::make_pair(TokenType::NUMERIC_LITERAL, tokenValue);
				}

				if (c == 'e' || c == 'E') {
					tokenValue += c;
					if (file_.get(c) && (c == '+' || c == '-' || std::isdigit(c))) {
						tokenValue += c;
						while (file_.get(c) && std::isdigit(c)) {
							tokenValue += c;
						}
						
						return std::make_pair(TokenType::NUMERIC_LITERAL, tokenValue);
					} else {
						return std::make_pair(TokenType::ERROR, tokenValue);
					}
				}

				return std::make_pair(TokenType::NUMERIC_LITERAL, tokenValue);
			} else if (std::isalpha(c) || c == '_') {
		        tokenValue += c;
		        while (file_.get(c) && (std::isalnum(c) || c == '_')) {
		            tokenValue += c;
		        }

		        if (std::regex_match(tokenValue, KEYWORD_REGEX)) {
		            return std::make_pair(TokenType::KEYWORD, tokenValue);
		        } else if (std::regex_match(tokenValue, INT_REGEX)) {
					return std::make_pair(TokenType::INT, tokenValue);
				} else if (std::regex_match(tokenValue, FLOAT_REGEX)) {
					return std::make_pair(TokenType::FLOAT, tokenValue);
				} else if (std::regex_match(tokenValue, STRING_REGEX)) {
					return std::make_pair(TokenType::STRING, tokenValue);
				} else if (std::regex_match(tokenValue, CHAR_REGEX)) {
					return std::make_pair(TokenType::CHAR, tokenValue);
				} else if (std::regex_match(tokenValue, BOOL_REGEX)) {
					return std::make_pair(TokenType::BOOL, tokenValue);
				} else if (std::regex_match(tokenValue, IDENTIFIER_REGEX)) {
		            return std::make_pair(TokenType::IDENTIFIER, tokenValue);
		        }
		    } else if (c == '=') {
				tokenValue += c;
				return std::make_pair(TokenType::ASSIGNMENT, tokenValue);
			} else if (c == ';') {
				tokenValue += c;
				return std::make_pair(TokenType::PUNCTUATION, tokenValue);
			} else if (isMathOperator(c)) {
				tokenValue += c;
				return std::make_pair(TokenType::MATH_OPERATOR, tokenValue);
			} else if (c == ',') {
				tokenValue += c;
				return std::make_pair(TokenType::COMMA, tokenValue);
			} else if (c == '(' || c == ')') {
				tokenValue += c;
				return std::make_pair(TokenType::ROUND_PAREN, tokenValue);
			} else if (c == '{' ||c == '}') {
				tokenValue += c;
				return std::make_pair(TokenType::CURLY_PAREN, tokenValue);
			} else if (c == '[' ||c == ']') {
				tokenValue += c;
				return std::make_pair(TokenType::SQUARE_PAREN, tokenValue);
			}

		    return std::make_pair(TokenType::NONE, "");
		}

		bool isMathOperator(char c) {
			return MATH_OPERATORS.find(c) != std::string::npos;
		}
		
		~Lexer() {
			file_.close();
		}
		
	private:
		std::string filename_;
		std::ifstream file_;
		int currentPos_;
		const std::string MATH_OPERATORS = "+-*/%^";
		const std::regex KEYWORD_REGEX{"if|else|while|for"};
		const std::regex IDENTIFIER_REGEX{"[a-zA-Z_][a-zA-Z0-9_]*"};
		const std::regex NUMERIC_LITERAL_REGEX{"\\d+"};
		const std::regex STRING_LITERAL_REGEX{"\"[^\"]*\""};
		const std::regex INT_REGEX{"int"};
		const std::regex FLOAT_REGEX{"float"};
		const std::regex STRING_REGEX{"string"};
		const std::regex CHAR_REGEX{"char"};
		const std::regex BOOL_REGEX{"bool"};
};

void compileFile(const std::string& filename) {
	Lexer lexer(filename);
	std::pair<TokenType, std::string> token;
	token = lexer.getNextToken();

	while(token.first != TokenType::END_OF_FILE && token.first != TokenType::ERROR) {
	    std::cout << token.first << " " << token.second << std::endl;
		token = lexer.getNextToken();
	}
}

int main() {
	compileFile("test.vg");

	return 0;
}