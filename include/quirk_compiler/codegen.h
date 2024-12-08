#ifndef CODEGEN_H
#define CODEGEN_H

#include "astnode.h"
#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include <variant>

class CodegenElement {
public:
  virtual ~CodegenElement() = default;
  virtual void print(int depth = 0) const = 0;
};

class Instruction : public CodegenElement {
public:
  std::string operation;
  std::string instruction;
  std::vector<std::shared_ptr<CodegenElement>> children;
  Instruction* parent_;

  Instruction(const std::string& op, const std::string& instr = "")
    : operation(op), instruction(instr) {}

  void insertAfter(std::string insertAfter, std::string toInsert) {
    size_t pos = instruction.find(insertAfter);

    if (pos != std::string::npos) {
      pos += insertAfter.length();

      instruction.insert(pos, toInsert);
    } else {
      std::cerr << "Could not find the substring to insert after!" << std::endl;
    }
  };

  void print(int depth = 0) const override {
    std::string indent(depth, ' ');
  
    if (!instruction.empty()) {
      std::cout << indent << instruction << std::endl;
    }

    for (const auto& child : children) {
      child->print(depth + 2);
    }
  }

  void addElement(std::shared_ptr<CodegenElement> element) {
    children.push_back(element);
  }
};

class Codegen {
public:
  Codegen() : temporaries_counter(0), labels_counter(0), parent_(nullptr) {}

  std::shared_ptr<Instruction> rootIR;
  std::shared_ptr<Instruction> current_parent;
  std::shared_ptr<Instruction> last_parent;

  void Init();
  void ConvertAST(ASTNode* ast);

  void printInstructions() { rootIR->print(); }
  std::shared_ptr<Instruction> findInstruction(std::shared_ptr<CodegenElement> root, std::shared_ptr<Instruction> isntr);
private:
  void dfsAST(ASTNode* node);
  std::string processNode(ASTNode* node, bool return_string);
  void convertCondition(ASTNode* node);
  std::vector<std::string> convertForCondition(ASTNode* node, std::string conditionLabel, std::string bodyLabel, std::string endLabel);
  std::string createTemporary() { return "%t" + std::to_string(temporaries_counter++); }
  std::string createLabel(std::string labelStart, int add) { return "%" + labelStart + std::to_string(labels_counter + add); }
  void switchParent(std::shared_ptr<Instruction> parent);
  void popParent();
  std::string toLowerCase(std::string string);

  int temporaries_counter = 0;
  int labels_counter = 0;
  Codegen* parent_;
  std::vector<std::pair<std::string, std::string>> identifierTable_;
};

#endif
