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

Codegen::Codegen() : builder(context), module(std::make_unique<Module>("quirk", context)) {
    // Create a test function
    FunctionType *funcType = FunctionType::get(builder.getVoidTy(), false);
    Function *mainFunc = Function::Create(funcType, Function::ExternalLinkage, "main", module.get());
    BasicBlock *entry = BasicBlock::Create(context, "entry", mainFunc);
    builder.SetInsertPoint(entry);
}

Module* Codegen::getModule() const {
    return module.get();
}

void Codegen::generateCode(ASTNode* root) {
    dfsGenerateCode(root);
    // Finalize the function
    builder.CreateRetVoid();
}

void Codegen::dfsGenerateCode(ASTNode* node) {
    processNode(node);

    const std::vector<ASTNode*>& children = node->getChildren();
    for (ASTNode* child : children) {
        dfsGenerateCode(child);
    }
}

void Codegen::processNode(ASTNode* node) {
    if (node->getType() == "LITERAL") {
        llvm::Value* value = generateExpression(node);
        if (value) {
            value->print(llvm::errs());
            std::cout << std::endl;
        }
    } else if (node->getType() == "VAR_DECLARATION") {
        std::string varName = node->getChildren()[1]->getValue(); // Assuming second child is the identifier
        ASTNode* varTypeNode = node->getChildren()[0]; // Assuming first child is the type node
        llvm::Type* varType = nullptr;
        if (varTypeNode->getValue() == "INT") {
            varType = llvm::Type::getInt32Ty(context);
        } else if (varTypeNode->getValue() == "FLOAT") {
            varType = llvm::Type::getFloatTy(context);
        } else if (varTypeNode->getValue() == "BOOL") {
            varType = llvm::Type::getInt1Ty(context);
        } else if (varTypeNode->getValue() == "CHAR") {
            varType = llvm::Type::getInt8Ty(context);
        } else if (varTypeNode->getValue() == "STRING") {
            varType = llvm::Type::getInt8Ty(context);
        }

        // Ensure we have a valid function context
        llvm::Function* function = builder.GetInsertBlock()->getParent();
        if (!function) {
            std::cerr << "Error: No valid function context available!" << std::endl;
            return;
        }

        llvm::AllocaInst* alloca = createEntryBlockAlloca(function, varName, varType);
        namedValues[varName] = alloca;

        if (node->getChildren().size() > 2) {
            ASTNode* assignmentNode = node->getChildren()[2];
            llvm::Value* value = generateExpression(assignmentNode->getChildren()[0]);
            builder.CreateStore(value, alloca);
        }
    } else if (node->getType() == "STATEMENT") {
        // Handle statements
    }
}

llvm::Value* Codegen::generateExpression(ASTNode* node) {
    std::string valueStr = node->getValue();
    if (node->getType() == "LITERAL") {
        if (valueStr == "true" || valueStr == "false") {
            bool value = (valueStr == "true");
            return llvm::ConstantInt::get(context, llvm::APInt(1, value));
        } else if (valueStr.front() == '"' && valueStr.back() == '"') {
            std::string strValue = valueStr.substr(1, valueStr.size() - 2);
            return builder.CreateGlobalStringPtr(strValue, "", 0, module.get());
        } else if (valueStr.front() == '\'' && valueStr.back() == '\'') {
            char charvalue = valueStr[1];
            return llvm::ConstantInt::get(context, llvm::APInt(8, charvalue));
        } else if (valueStr.find('.') != std::string::npos) {
            float value = std::stof(valueStr);
            return llvm::ConstantFP::get(context, llvm::APFloat(value));
        } else {
            uint64_t value = std::stoull(valueStr);
            return llvm::ConstantInt::get(context, llvm::APInt(32, value));
        }
    }
    return nullptr;
}

llvm::AllocaInst* Codegen::createEntryBlockAlloca(llvm::Function* function, const std::string& varName, llvm::Type* type) {
    llvm::IRBuilder<> tmpB(&function->getEntryBlock(), function->getEntryBlock().begin());
    return tmpB.CreateAlloca(type, 0, varName.c_str());
}

/*
Type: Program, Value:
  Type: VAR_DECLARATION, Value:
    Type: VAR_TYPE, Value: STRING
    Type: IDENTIFIER, Value: _string
    Type: ASSIGNMENT, Value:
      Type: LITERAL, Value: "Hello World!"
  Type: VAR_DECLARATION, Value:
    Type: VAR_TYPE, Value: CHAR
    Type: IDENTIFIER, Value: _char
    Type: ASSIGNMENT, Value:
      Type: LITERAL, Value: 'j'
  Type: VAR_DECLARATION, Value:
    Type: VAR_TYPE, Value: INT
    Type: IDENTIFIER, Value: _int
    Type: ASSIGNMENT, Value:
      Type: LITERAL, Value: 10
  Type: VAR_DECLARATION, Value:
    Type: VAR_TYPE, Value: FLOAT
    Type: IDENTIFIER, Value: _float
    Type: ASSIGNMENT, Value:
      Type: LITERAL, Value: 10.5
  Type: VAR_DECLARATION, Value:
    Type: VAR_TYPE, Value: BOOL
    Type: IDENTIFIER, Value: _bool
    Type: ASSIGNMENT, Value:
      Type: LITERAL, Value: false
  Type: STATEMENT, Value: if
    Type: CONDITION, Value: _temp=="Hello World"
    Type: CODE_BLOCK, Value:
      Type: STATEMENT, Value: out
        Type: FUNCTIONCALL, Value: out
          Type: LITERAL/IDENTIFIER, Value: "Temp is Hello World"
      Type: STATEMENT, Value: if
        Type: CONDITION, Value: _temp==1
        Type: CODE_BLOCK, Value:
          Type: STATEMENT, Value: out
            Type: FUNCTIONCALL, Value: out
              Type: LITERAL/IDENTIFIER, Value: "Temp is 1"
      Type: STATEMENT, Value: else
        Type: CODE_BLOCK, Value:
          Type: STATEMENT, Value: out
            Type: FUNCTIONCALL, Value: out
              Type: LITERAL/IDENTIFIER, Value: "Hello World"
      Type: STATEMENT, Value: while
        Type: CONDITION, Value: _temp!=9
        Type: CODE_BLOCK, Value:
          Type: STATEMENT, Value: out
            Type: FUNCTIONCALL, Value: out
              Type: LITERAL/IDENTIFIER, Value: "Temp"
  Type: STATEMENT, Value: while
    Type: CONDITION, Value: _temp!=5
    Type: CODE_BLOCK, Value:
      Type: STATEMENT, Value: out
        Type: FUNCTIONCALL, Value: out
          Type: LITERAL/IDENTIFIER, Value: "Loop"
  Type: STATEMENT, Value: for
    Type: CONDITON, Value: inti=0;i<5;i++
    Type: CODE_BLOCK, Value:
    Type: CODE_BLOCK, Value:
      Type: STATEMENT, Value: out
        Type: FUNCTIONCALL, Value: out
          Type: LITERAL/IDENTIFIER, Value: "Test"
*/
