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
    processNode(node);
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

  if (nodeType == "VAR_DECLARATION") {
    std::string varName = node->getChildren()[1]->getValue();
    ASTNode* assignmentNode = node->getChildren()[2];

    std::string literalValue = assignmentNode->getChildren()[0]->getValue();
    std::string tempVar = createTemporary();

    addInstruction(Instruction("assign", tempVar, literalValue));
    addInstruction(Instruction("assign", varName, tempVar));
  } else if (nodeType == "STATEMENT" && node->getValue() == "if") {

  } else if (nodeType == "STATEMENT" && node->getValue() == "out") {

  }
}

void Codegen::printInstructions() const {
  for (const auto& instr : instructions) {
    std::cout << instr.toString() << std::endl;
  }
}
