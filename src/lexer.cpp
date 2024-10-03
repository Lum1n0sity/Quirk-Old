#include "lexer.h"

Lexer::Lexer(const std::string &filename)
    : filename_(filename), currentPos_(0), lineNumber_(1),
      MATH_OPERATORS("+-*/%^"),
      KEYWORD_REGEX("if|else|while|for|function|out"),
      IDENTIFIER_REGEX("[a-zA-Z_][a-zA-Z0-9_]*"), NUMERIC_LITERAL_REGEX("\\d+"),
      STRING_LITERAL_REGEX("\"[^\"]*\""), CHAR_LITERAL_REGEX("\'[^\']*\'"),
      BOOL_LITERAL_REGEX("true|false"), INT_REGEX("int"), FLOAT_REGEX("float"),
      STRING_REGEX("string"), CHAR_REGEX("char"), BOOL_REGEX("bool"),
      RELATIONAL_OPERATOR_REGEX("==|!=|<|>|<=|>="),
      UNARY_ARITHMETIC_OPERATOR_REGEX("\\+\\+|--") {
  file_.open(filename_, std::ios::in);
  if (!file_.is_open()) {
    std::cerr << "Error: Could not open file " << filename_ << std::endl;
  }
}

std::pair<TokenType, std::string> Lexer::getNextToken() {
  std::string tokenValue;
  char c;

  // Skip whitespace and newlines
  while (file_.get(c) && std::isspace(c)) {
    if (c == '\n') {
      lineNumber_++;
      currentPos_ = 0;
    }
  }

  // End of file
  if (file_.eof()) {
    return std::make_pair(TokenType::END_OF_FILE, "");
  }

  // Handle single character tokens
  if (c == '(' || c == ')') {
    currentPos_++;
    return std::make_pair(TokenType::ROUND_PAREN, std::string(1, c));
  } else if (c == '{' || c == '}') {
    return std::make_pair(TokenType::CURLY_PAREN, std::string(1, c));
  } else if (c == '[' || c == ']') {
    return std::make_pair(TokenType::SQUARE_PAREN, std::string(1, c));
  } else if (c == ';') {
    currentPos_++;
    return std::make_pair(TokenType::PUNCTUATION, std::string(1, c));
  } else if (c == ',') {
    currentPos_++;
    return std::make_pair(TokenType::COMMA, std::string(1, c));
  }

  // Handle string literals
  if (c == '"') {
    tokenValue += c;
    while (file_.get(c) && c != '"') {
      tokenValue += c;
    }
    tokenValue += c;
    currentPos_ += tokenValue.size();
    return std::make_pair(TokenType::STRING_LITERAL, tokenValue);
  }

  // Handle character literals
  if (c == '\'') {
    tokenValue += c;
    while (file_.get(c) && c != '\'') {
      tokenValue += c;
    }
    tokenValue += c;
    currentPos_ += tokenValue.size();
    if (std::regex_match(tokenValue, CHAR_LITERAL_REGEX)) {
      return std::make_pair(TokenType::CHAR_LITERAL, tokenValue);
    } else {
      return std::make_pair(TokenType::ERROR, tokenValue);
    }
  }

  // Handle numeric literals
  if (std::isdigit(c)) {
    tokenValue += c;
    while (file_.get(c) && std::isdigit(c)) {
      tokenValue += c;
    }
    file_.unget();
    if (file_.peek() == '.') {
      tokenValue += '.';
      file_.get(c);
      while (file_.get(c) && std::isdigit(c)) {
        tokenValue += c;
      }
      currentPos_ += tokenValue.size();
      return std::make_pair(TokenType::NUMERIC_LITERAL, tokenValue);
    }
    if (c == 'e' || c == 'E') {
      tokenValue += c;
      if (file_.get(c) && (c == '+' || c == '-' || std::isdigit(c))) {
        tokenValue += c;
        while (file_.get(c) && std::isdigit(c)) {
          tokenValue += c;
        }
        file_.unget();
        currentPos_ += tokenValue.size();
        return std::make_pair(TokenType::NUMERIC_LITERAL, tokenValue);
      } else {
        currentPos_ += tokenValue.size();
        return std::make_pair(TokenType::ERROR, tokenValue);
      }
    }
    return std::make_pair(TokenType::NUMERIC_LITERAL, tokenValue);
  }

  // Handle identifiers, keywords and variable types
  if (std::isalpha(c) || c == '_') {
    tokenValue += c;
    while (file_.get(c) && (std::isalnum(c) || c == '_')) {
      tokenValue += c;
    }
    
    if (c == ' ') {
      std::string nextToken;
      while (file_.get(c) && c == ' ');
      if (c == 'i') {
        nextToken += c;
        while (file_.get(c) && (std::isalnum(c) || c == '_')) {
          nextToken += c;
        }
        file_.unget();

        if (nextToken == "if") {
          tokenValue += " " + nextToken;
        }
      } else {
        file_.unget();
      }
    } else {
      file_.unget();
    }
  
    currentPos_ += tokenValue.size();

    std::cout << tokenValue << std::endl;

    // Check for variable types
    if (std::regex_match(tokenValue, INT_REGEX)) {
      return std::make_pair(TokenType::INT, tokenValue);
    } else if (std::regex_match(tokenValue, FLOAT_REGEX)) {
      return std::make_pair(TokenType::FLOAT, tokenValue);
    } else if (std::regex_match(tokenValue, STRING_REGEX)) {
      return std::make_pair(TokenType::STRING, tokenValue);
    } else if (std::regex_match(tokenValue, CHAR_REGEX)) {
      return std::make_pair(TokenType::CHAR, tokenValue);
    } else if (std::regex_match(tokenValue, BOOL_REGEX)) {
      return std::make_pair(TokenType::BOOL, tokenValue);
    }

    // Check for keywords
    if (tokenValue == "else if") {
      return std::make_pair(TokenType::KEYWORD, tokenValue);
    } else if (std::regex_match(tokenValue, KEYWORD_REGEX)) {
      return std::make_pair(TokenType::KEYWORD, tokenValue);
    }

    // Check for identifiers
    if (std::regex_match(tokenValue, IDENTIFIER_REGEX)) {
      return std::make_pair(TokenType::IDENTIFIER, tokenValue);
    } else {
      return std::make_pair(TokenType::ERROR, tokenValue);
    }
  }

  // Handle relational operators
  if (c == '=' || c == '!' || c == '<' || c == '>') {
    tokenValue += c;
    if (file_.get(c) && c == '=') {
      tokenValue += c;
      currentPos_ += tokenValue.size();
      return std::make_pair(TokenType::RELATIONAL_OPERATOR, tokenValue);
    } else {
      file_.unget();
      if (tokenValue == "=") {
        currentPos_ += tokenValue.size();
        return std::make_pair(TokenType::ASSIGNMENT, tokenValue);
      } else if (tokenValue == "!") {
        currentPos_ += tokenValue.size();
        return std::make_pair(TokenType::ERROR, tokenValue);
      } else {
        currentPos_ += tokenValue.size();
        return std::make_pair(TokenType::RELATIONAL_OPERATOR, tokenValue);
      }
    }
  }

  // Handle math operators
  if (isMathOperator(c)) {
    tokenValue += c;
    while (file_.get(c) && isMathOperator(c)) {
      tokenValue += c;
    }
    file_.unget();
    if (tokenValue.size() > 1) {
      currentPos_ += tokenValue.size();
      return std::make_pair(TokenType::UNARY_ARITHMETIC_OPERATOR, tokenValue);
    } else {
      currentPos_ += tokenValue.size();
      return std::make_pair(TokenType::MATH_OPERATOR, tokenValue);
    }
  }

  // If none of the above, return an error
  return std::make_pair(TokenType::ERROR, std::string(1, c));
}

bool Lexer::isMathOperator(char c) {
  return MATH_OPERATORS.find(c) != std::string::npos;
}

std::string Lexer::getCurrentLineNumber() const {
  return std::to_string(lineNumber_) + "; " + std::to_string(currentPos_);
}

Lexer::~Lexer() { file_.close(); }
