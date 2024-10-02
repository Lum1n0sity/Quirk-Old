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
    std::string initialization;
    std::string condition;
    std::string update;
    bool error;
};

#endif
