#ifndef CODEGEN_H
#define CODEGEN_H

#include "astnode.h"
#include <memory>
#include <string>
#include <vector>

enum CodegenType {
  FUNCTION,
  VARIABLE,
  LITERAL,
  BINARY_OP,
  UNARY_OP,
  ASSIGNMENT_CT,
  CONTROL_FLOW,
};

class Codegen {
public:
  Codegen() : type(FUNCTION), value(""), parent_(nullptr), processed_(false) {}

  Codegen(CodegenType type, std::string value = "", bool processed_ = false)
    : type(type), value(value), parent_(nullptr), processed_(processed_) {};

  void add_child(std::shared_ptr<Codegen> node) {
    children.push_back(node);
  };

  void set_parent(std::shared_ptr<Codegen> parent) {
    parent_ = parent;
  };

  void set_type(CodegenType newType) {
    type = newType;
  };

  void set_value(const std::string& newValue) {
    value = newValue;
  };

  std::shared_ptr<Codegen> get_parent() const {
    return parent_;
  }

  CodegenType get_type() const {
    return type;
  }

  std::string get_value() const {
    return value;
  }

  const std::vector<std::shared_ptr<Codegen>>& get_children() const {
    return children;  
  }

  void ConvertAST(ASTNode* ast);

  bool isProcessed() const { return processed_; }
  void setProcessed(bool processed) { processed_ = processed; }

private:
  void dfsAST(ASTNode* node);
  void processNode(ASTNode* node);

  CodegenType type;
  std::string value;
  std::vector<std::shared_ptr<Codegen>> children;
  std::shared_ptr<Codegen> parent_;
  bool processed_;
};

#endif
