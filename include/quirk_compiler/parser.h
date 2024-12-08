#ifndef PARSER_H
#define PARSER_H

#include "astnode.h"
#include "condition.h"
#include "lexer.h"
#include <iostream>
#include <string>
#include <vector>

class Parser {
public:
  Parser(const std::string &filename);

  void Initalize();
  ASTNode *parse();
  Condition parseCondition();
  ForLoopCondition parseForLoopCondition();

private:
  Lexer lexer_;
  ASTNode *current_parent_;
  std::vector<ASTNode *> scope_stack_;
  std::vector<std::string> uniqueNameList_;

  bool parseVarAssignment(TokenType varLiteralType, std::string varType);
  std::tuple<bool, std::string, std::string> isNextTokenLiteralOrIdentifier();
  std::string tokenTypeToString(TokenType tokenType);
  void switchParentNode(ASTNode *new_parent);
  void popParentNode();
};

void printAST(ASTNode *node, int depth = 0);

#endif
