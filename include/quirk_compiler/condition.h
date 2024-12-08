#ifndef CONDITION_H
#define CONDITION_H

#include <string>
#include "lexer.h"

struct Condition {
  std::pair<TokenType, std::string> left;
  std::pair<TokenType, std::string> op;
  std::pair<TokenType, std::string> right;
  bool error;
};

struct ForLoopCondition {
  std::pair<TokenType, std::string> cInt;
  std::pair<TokenType, std::string> i;
  std::pair<TokenType, std::string> nl;
  std::pair<TokenType, std::string> i2;
  std::pair<TokenType, std::string> ro;
  std::pair<TokenType, std::string> len;
  std::pair<TokenType, std::string> i3;
  std::pair<TokenType, std::string> uao;
  bool error;
};

#endif
