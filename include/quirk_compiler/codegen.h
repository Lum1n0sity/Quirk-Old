#ifndef CODEGEN_H
#define CODEGEN_H

#include "astnode.h"
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <memory>
#include <unordered_map>
#include <string>

class Codegen {
public:
    Codegen();
    llvm::Module* getModule() const;
    void generateCode(ASTNode* root);
      
    std::vector<llvm::BasicBlock *> thenBlocks;
    std::vector<llvm::BasicBlock *> elseBlocks;
    std::vector<llvm::BasicBlock *> mergeBlocks;
private:
    llvm::LLVMContext context;
    llvm::IRBuilder<> builder;
    std::unique_ptr<llvm::Module> module;
    std::unordered_map<std::string, llvm::Value*> namedValues;
    llvm::Function* outFunc;

    void dfsGenerateCode(ASTNode* node);
    void processNode(ASTNode* node);
    llvm::Value* generateExpression(ASTNode* node);
    llvm::AllocaInst* createEntryBlockAlloca(llvm::Function* function, const std::string& varName, llvm::Type* type);
    std::string removeSuffix(const std::string& str, std::string suffix);
//    llvm::Value* generateCondition(ASTNode* node);

    std::unordered_map<std::string, llvm::Constant*> stringLiterals;
    std::unordered_map<std::string, llvm::Constant*> charLiterals;

    // Debug
    void printStringLiterals() const;
};

#endif
