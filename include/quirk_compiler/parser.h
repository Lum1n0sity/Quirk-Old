#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <vector>
#include <iostream>
#include "lexer.h"
#include "astnode.h"
#include "condition.h"

class Parser {
public:
    Parser(const std::string& filename);

    void Initalize();
    ASTNode* parse();
    Condition parseCondition();
    ForLoopCondition parseForLoopCondition();

private:
    Lexer lexer_;
    ASTNode* current_parent_;
    std::vector<ASTNode*> scope_stack_;

    bool parseVarAssignment(TokenType varLiteralType, std::string varType);
    std::pair<bool, std::string> isNextTokenLiteralOrIdentifier();
    void switchParentNode(ASTNode* new_parent);
    void popParentNode();
};

void printAST(ASTNode* node, int depth = 0);

#endif