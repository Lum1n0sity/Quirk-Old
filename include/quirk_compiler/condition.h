#ifndef CONDITION_H
#define CONDITION_H

#include <string>

struct Condition {
    std::string part1;
    std::string op;
    std::string part2;
    bool error;
};

struct ForLoopCondition {
    std::string initialization;
    std::string condition;
    std::string update;
    bool error;
};

#endif
