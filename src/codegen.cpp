#include <iostream>
#include "astnode.h"
#include "codegen.h"

void Codegen::ConvertAST(ASTNode* ast) {
  if (ast != nullptr) {
    dfsAST(ast);
  } else {
    std::cerr << "AST is null!" << std::endl;
    std::terminate();
  }
}

void Codegen::dfsAST(ASTNode* node) {
  if (node == nullptr) { std::cerr << "No nodes for dfs!" << std::endl; std::terminate(); }

  if (node->getType() != "Program") {
    node->setProcessed(true);
    std::cout << node->getType() << node->getValue() << std::endl;
    for (const auto& child: node->getChildren()) {
      dfsAST(child);
    }
  } else {
    for (const auto& child : node->getChildren()) {
      dfsAST(child);
    }
  }
}

void Codegen::processNode(ASTNode* node) {
  std::string nodeType = node->getType();
}
