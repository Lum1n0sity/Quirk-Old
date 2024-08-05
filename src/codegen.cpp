#include <llvm/IR/Constants.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/ADT/APInt.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Type.h>
#include <iostream>
#include "astnode.h"
#include "codegen.h"

using namespace llvm;

// Initalize IRBuilder, model, main function and other std lib functions
Codegen::Codegen() : builder(context), module(std::make_unique<Module>("quirk", context))
{
  // Create test function
  FunctionType *funcType = FunctionType::get(builder.getVoidTy(), false);
  Function *mainFunc = Function::Create(funcType, Function::ExternalLinkage, "main", module.get());
  BasicBlock *entry = BasicBlock::Create(context, "entry", mainFunc);
  builder.SetInsertPoint(entry);

  // Declare out() function
  std::vector<llvm::Type*> outArgs;
  outArgs.push_back(builder.getInt8Ty());
  FunctionType *outFuncType = FunctionType::get(builder.getVoidTy(), outArgs, false);
  outFunc = Function::Create(outFuncType, Function::ExternalLinkage, "out", module.get());
}

// Return current generated model
Module *Codegen::getModule() const
{
  return module.get();
}

// Initalize code generation
void Codegen::generateCode(ASTNode *root)
{
  dfsGenerateCode(root);
 
  builder.CreateRetVoid();

  // Debug
  // printStringLiterals();
}

// Travers through Abstract Syntax Tree
void Codegen::dfsGenerateCode(ASTNode *node)
{
  processNode(node);

  const std::vector<ASTNode *> &children = node->getChildren();
  for (ASTNode *child : children)
  {
    dfsGenerateCode(child);
  }
}

// Converts the current ASTNode to LLVM IR
void Codegen::processNode(ASTNode *node)
{
  if (node->getType() == "LITERAL")
  {
    llvm::Value *value = generateExpression(node);
  }
  else if (node->getType() == "VAR_DECLARATION")
  {
    std::string varName = node->getChildren()[1]->getValue();
    ASTNode *varTypeNode = node->getChildren()[0];
    llvm::Type *varType = nullptr;
    if (varTypeNode->getValue() == "INT")
    {
      varType = llvm::Type::getInt32Ty(context);
    }
    else if (varTypeNode->getValue() == "FLOAT")
    {
      varType = llvm::Type::getFloatTy(context);
    }
    else if (varTypeNode->getValue() == "BOOL")
    {
      varType = llvm::Type::getInt1Ty(context);
    }
    else if (varTypeNode->getValue() == "CHAR")
    {
      varType = llvm::Type::getInt8Ty(context);
    }
    else if (varTypeNode->getValue() == "STRING")
    {
      varType = llvm::Type::getInt8Ty(context);
    }

    llvm::Function *function = builder.GetInsertBlock()->getParent();
    if (!function)
    {
      std::cerr << "Error: No valid function context available!" << std::endl;
      return;
    }

    llvm::AllocaInst *alloca = createEntryBlockAlloca(function, varName, varType);
    namedValues[varName] = alloca;

    if (node->getChildren().size() > 2)
    {
      ASTNode *assignmentNode = node->getChildren()[2];
      llvm::Value *value = generateExpression(assignmentNode->getChildren()[0]);
      builder.CreateStore(value, alloca);
    }
  }
  else if (node->getType() == "STATEMENT" && node->getValue() == "if")
  {
    ASTNode *conditionNode = node->getChildren()[0];
    ASTNode *thenBlockNode = node->getChildren()[1];

    llvm::Value *condition = generateExpression(conditionNode);

    Function *function = builder.GetInsertBlock()->getParent();
    BasicBlock *thenBB = BasicBlock::Create(context, "then", function);
    BasicBlock *elseBB = BasicBlock::Create(context, "else");
    BasicBlock *mergeBB = BasicBlock::Create(context, "ifcont");

    thenBlocks.push_back(thenBB);
    elseBlocks.push_back(elseBB);
    mergeBlocks.push_back(mergeBB);

    builder.CreateCondBr(condition, thenBB, elseBB);

    builder.SetInsertPoint(thenBB);
    dfsGenerateCode(thenBlockNode);
    builder.CreateBr(mergeBB);

    builder.SetInsertPoint(elseBB);
    if (node->getChildren().size() > 2)
    {
      ASTNode *elseBlockNode = node->getChildren()[2];
      dfsGenerateCode(elseBlockNode);
    }
    builder.CreateBr(mergeBB);

    builder.SetInsertPoint(mergeBB);
  } else if (node->getType() == "STATEMENT" && node->getValue() == "out") {
    ASTNode *funcCallNode = node->getChildren()[0];
    ASTNode *argNode = funcCallNode->getChildren()[0];
    argNode->set_type("LITERAL");

    llvm::Value *argValue = generateExpression(argNode);

    std::vector<llvm::Value*> args; 
    args.push_back(argValue);
    builder.CreateCall(outFunc, args);
  }
}

// Evaluates literals of different types and returns the coresponding LLVM constant
llvm::Value *Codegen::generateExpression(ASTNode *node)
{
  std::string valueStr = node->getValue();
  if (node->getType() == "LITERAL")
  {
    if (valueStr == "true" || valueStr == "false")
    {
      bool value = (valueStr == "true");
      return llvm::ConstantInt::get(context, llvm::APInt(1, value));
    }
    else if (valueStr.front() == '"' && valueStr.back() == '"')
    {
      std::string strValue = valueStr.substr(1, valueStr.size() - 2);
  
      // Check if the string literal already exists
      if (stringLiterals.find(strValue) != stringLiterals.end()) {
        return stringLiterals[strValue];
      }
    
      // Create new constant
      llvm::Constant *constant = builder.CreateGlobalStringPtr(strValue, "", 0, module.get());
      stringLiterals[strValue] = constant;
      return constant;
    }
    else if (valueStr.front() == '\'' && valueStr.back() == '\'')
    {
      char charvalue = valueStr[1];
      
      // Check if the char literal already exists
      std::string charStr(1, charvalue);
      if (charLiterals.find(charStr) != charLiterals.end()) {
        return charLiterals[charStr];
      }

      // Create new constant
      llvm::Constant *constant = llvm::ConstantInt::get(context, llvm::APInt(8, charvalue));
      charLiterals[charStr] = constant;
      return constant;
    }
    else if (valueStr.find('.') != std::string::npos)
    {
      float value = std::stof(valueStr);
      return llvm::ConstantFP::get(context, llvm::APFloat(value));
    }
    else
    {
      uint64_t value = std::stoull(valueStr);
      return llvm::ConstantInt::get(context, llvm::APInt(32, value));
    }
  }

  return nullptr;
}

// Allocates memory for each varibale
llvm::AllocaInst *Codegen::createEntryBlockAlloca(llvm::Function *function, const std::string &varName, llvm::Type *type)
{
  llvm::IRBuilder<> tmpB(&function->getEntryBlock(), function->getEntryBlock().begin());
  return tmpB.CreateAlloca(type, 0, varName.c_str());
}

// Removes a specified suffix from a string
std::string Codegen::removeSuffix(const std::string& str, std::string suffix) {
  if (str.size() >= suffix.size() && str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0) {
    return str.substr(0, str.size() - suffix.size());
  }

  return str;
}

void Codegen::printStringLiterals() const {
  std::cout << "String Literals:" << std::endl;
  for (const auto &pair : stringLiterals) {
    std::cout << "String: " << pair.first << " -> ";
    pair.second->print(llvm::errs());
    std::cout << std::endl;
  }

  std::cout << "Char Literals:" << std::endl;
  for (const auto &pair : charLiterals) {
    std::cout << "Char: " << pair.first << " -> ";
    pair.second->print(llvm::errs());
    std::cout << std::endl;
  }
}
