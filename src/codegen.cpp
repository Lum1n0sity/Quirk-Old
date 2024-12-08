#include "codegen.h"
#include "astnode.h"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <string>
#include <vector>

void Codegen::Init() {
  rootIR = std::make_shared<Instruction>("root");
  current_parent = rootIR;
}

void Codegen::ConvertAST(ASTNode *ast) {
  Init();
  if (ast != nullptr) {
    dfsAST(ast);
  } else {
    std::cerr << "AST is null!" << std::endl;
    std::terminate;
  }
}

void Codegen::dfsAST(ASTNode *node) {
  if (node == nullptr) {
    std::cerr << "No nodes for dfs!" << std::endl;
    std::terminate();
  }

  if (node->isProcessed()) {
    return;
  }

  node->setProcessed(true);

  if (node->getType() != "Program") {
    processNode(node, false);
    for (const auto &child : node->getChildren()) {
      dfsAST(child);
    }
  } else {
    for (const auto &child : node->getChildren()) {
      dfsAST(child);
    }
  }
}

std::string Codegen::processNode(ASTNode *node, bool return_string) {
  std::string nodeType = node->getType();

  if (nodeType == "VAR_DECLARATION") {
    std::string temporary = createTemporary();
    std::string varName = node->getChildren()[1]->getValue();
    std::string literalValue =
        node->getChildren()[2]->getChildren()[0]->getValue();
    std::string varType = toLowerCase(node->getChildren()[0]->getValue());
    std::transform(varType.begin(), varType.end(), varType.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    if (!return_string) {
      current_parent->addElement(std::make_shared<Instruction>(
          "alloc", temporary + " = alloc " + varType));
      current_parent->addElement(std::make_shared<Instruction>(
          "store", "store " + literalValue + ", " + temporary));
      identifierTable_.push_back(std::make_pair(temporary, varName));
    }
  } else if (nodeType == "STRING_LITERAL") {
    std::string valueString = node->getValue();

    if (return_string) {
      return valueString;
    }
  } else if (nodeType == "CHAR_LITERAL") {
    std::string valueChar = node->getValue();

    if (return_string) {
      return valueChar;
    }
  } else if (nodeType == "NUMERIC_LITERAL") {
    std::string valueNumeric = node->getValue();

    if (return_string) {
      return valueNumeric;
    }
  } else if (nodeType == "BOOL_LITERAL") {
    std::string valueBool = node->getValue();

    if (return_string) {
      return valueBool;
    }
  } else if (nodeType == "STATEMENT" && node->getValue() == "if") {
    convertCondition(node->getChildren()[0]);

    std::string conditionTemp = "%t" + std::to_string(temporaries_counter - 1);
    std::string thenLabel = createLabel("then", 0);
    std::string elseifLabel = createLabel("elseif", 0);
    std::string elseifBlockLabel = createLabel("then", 1);
    std::string elseLabel = createLabel("else", 0);
    std::string mergeLabel = createLabel("merge", 0);

    std::shared_ptr<Instruction> brInstruction = std::make_shared<Instruction>(
        "br", "br " + conditionTemp + ", label " + thenLabel);
    current_parent->addElement(brInstruction);

    std::shared_ptr<Instruction> thenLabelIR =
        std::make_shared<Instruction>("label", thenLabel);
    current_parent->addElement(thenLabelIR);
    switchParent(thenLabelIR);
    ASTNode *thenBlock = node->getChildren()[1];
    thenBlock->setProcessed(true);
    processNode(thenBlock, false);

    current_parent->addElement(
        std::make_shared<Instruction>("br", "br label " + mergeLabel));
    popParent();

    // Check for else if and else nodes
    ASTNode *parent = node->get_parent();
    if (parent == nullptr) {
      std::cerr << "parent is null" << std::endl;
      std::terminate();
    }

    std::vector<ASTNode *> siblings = parent->getChildren();

    bool elseNodeFound = false;
    bool elseifNodeFound = false;

    ASTNode *elseifNode;
    ASTNode *elseNode;

    for (int i = 0; i < siblings.size(); i++) {
      if (siblings[i] == node) {
        // Check the sibling right after the current 'if'
        if (i + 1 < siblings.size()) {
          ASTNode *siblingAfterIfNode = siblings[i + 1];
          if (siblingAfterIfNode != nullptr) {
            if (siblingAfterIfNode->getValue() == "else if") {
              elseifNode = siblingAfterIfNode;
              elseifNodeFound = true;
              if (siblings[i + 2] != nullptr) {
                if (siblings[i + 2]->getValue() == "else") {
                  elseNode = siblings[i + 2];
                  elseNodeFound = true;
                  break;
                }
              }
              break;
            } else if (siblingAfterIfNode->getValue() == "else") {
              elseNode = siblingAfterIfNode;
              elseNodeFound = true;
              break;
            }
          }
        }
        break; // No need to check further once the current 'if' is found
      }
    }

    std::shared_ptr<Instruction> updatedBrInstruction;

    if (elseifNodeFound && elseifNode != nullptr) {
      std::shared_ptr<Instruction> elseifSectionLabel =
          std::make_shared<Instruction>("label", elseifLabel);
      current_parent->addElement(elseifSectionLabel);

      ASTNode *elseifCondition = elseifNode->getChildren()[0];
      convertCondition(elseifCondition);

      std::string insertAfter = " label " + thenLabel;
      updatedBrInstruction = findInstruction(rootIR, brInstruction);
      updatedBrInstruction->insertAfter(insertAfter, ", label " + elseifLabel);

      std::string conditionTempElseif =
          "%t" + std::to_string(temporaries_counter - 1);
      std::shared_ptr<Instruction> brInstructionElseif =
          std::make_shared<Instruction>(
              "br", "br " + conditionTempElseif + ", label " +
                        elseifBlockLabel + ", label " + elseLabel);
      current_parent->addElement(brInstructionElseif);

      std::shared_ptr<Instruction> elseifLabelIR =
          std::make_shared<Instruction>("label ", elseifBlockLabel);
      current_parent->addElement(elseifLabelIR);
      switchParent(elseifLabelIR);

      ASTNode *elseifBlock = elseifNode->getChildren()[1];
      elseifBlock->setProcessed(true);
      processNode(elseifBlock, false);

      current_parent->addElement(
          std::make_shared<Instruction>("br", "br label " + mergeLabel));
      popParent();
    }

    if (elseNodeFound && elseNode != nullptr) {
      std::shared_ptr<Instruction> elseBrInstruction;

      std::string insertAfter = "";
      if (elseifNodeFound) {
        insertAfter = " label " + elseifLabel;
        elseBrInstruction = updatedBrInstruction;
      } else {
        insertAfter = " label " + thenLabel;
        elseBrInstruction = brInstruction;
      }

      if (!insertAfter.empty()) {
        std::shared_ptr<Instruction> storedBrInstruction =
            findInstruction(rootIR, elseBrInstruction);
        storedBrInstruction->insertAfter(insertAfter, ", label " + elseLabel);

        std::shared_ptr<Instruction> elseLabelIR =
            std::make_shared<Instruction>("label ", elseLabel);
        current_parent->addElement(elseLabelIR);
        switchParent(elseLabelIR);

        ASTNode *elseBlock = elseNode->getChildren()[0];
        elseBlock->setProcessed(true);
        processNode(elseBlock, false);

        current_parent->addElement(
            std::make_shared<Instruction>("br", "br label " + mergeLabel));
        popParent();
      }
    }

    // Add merge
    current_parent->addElement(
        std::make_shared<Instruction>("label", mergeLabel));
  } else if (nodeType == "STATEMENT" && node->getValue() == "for") {
    std::string loopConditionLabel = createLabel("for_loop", 0);
    std::string loopBodyLabel = createLabel("loop_body", 0);
    std::string loopEndLabel = createLabel("loop_end", 0);
    std::vector<std::string> conditionTemps = convertForCondition(
        node->getChildren()[0], loopConditionLabel, loopBodyLabel, loopEndLabel);

    std::shared_ptr<Instruction> loopBodyLabelIR =
        std::make_shared<Instruction>("label", loopBodyLabel);
    current_parent->addElement(loopBodyLabelIR);
    switchParent(loopBodyLabelIR);

    ASTNode *loopBody = node->getChildren()[1];
    loopBody->setProcessed(true);
    processNode(loopBody, false);

    // Increment / Decrement condition counter
    std::string uao = node->getChildren()[0]->getChildren()[7]->getValue();
    if (uao == "++") {
      current_parent->addElement(std::make_shared<Instruction>(
          "inc", conditionTemps[0] + " = " + conditionTemps[0] + " + 1"));
    } else if (uao == "--") {
      current_parent->addElement(std::make_shared<Instruction>(
          "dec", conditionTemps[0] + " = " + conditionTemps[0] + " - 1"));
    }

    // Update original counter variable
    current_parent->addElement(std::make_shared<Instruction>(
        "store", "store " + conditionTemps[0] + ", " + conditionTemps[1]));

    // Break label to %for_loop
    current_parent->addElement(std::make_shared<Instruction>("br", "br label " + loopConditionLabel));
    popParent();

    // Loop end label
    current_parent->addElement(std::make_shared<Instruction>("label", loopEndLabel));
  } else if (nodeType == "CODE_BLOCK") {
    std::vector<ASTNode *> codeBlockChildren = node->getChildren();
    for (int i = 0; i < codeBlockChildren.size(); i++) {
      codeBlockChildren[i]->setProcessed(true);
      processNode(codeBlockChildren[i], false);
    }
  }

  return "";
}

void Codegen::convertCondition(ASTNode *node) {
  std::string leftTemp, rightTemp, operatorTemp;

  // Convert left operand
  if (node->getChildren()[0]->getType() == "IDENTIFIER") {
    std::string var = node->getChildren()[0]->getValue();

    if (auto it = std::find_if(
            identifierTable_.begin(), identifierTable_.end(),
            [&var](const auto &pair) { return pair.second == var; });
        it != identifierTable_.end()) {
      leftTemp = it->first;
    }
  } else {
    node->getChildren()[0]->setProcessed(true);
    leftTemp = processNode(node->getChildren()[0], true);
  }

  operatorTemp = node->getChildren()[1]->getValue();

  // Convert right operand
  if (node->getChildren()[2]->getType() == "IDENTIFIER") {
    std::string var = node->getChildren()[2]->getValue();

    if (auto it = std::find_if(
            identifierTable_.begin(), identifierTable_.end(),
            [&var](const auto &pair) { return pair.second == var; });
        it != identifierTable_.end()) {
      rightTemp = it->first;
    }
    rightTemp = node->getChildren()[2]->getValue();
  } else {
    node->getChildren()[2]->setProcessed(true);
    rightTemp = processNode(node->getChildren()[2], true);
  }

  std::string tempVar = createTemporary();

  if (operatorTemp == "==") {
    current_parent->addElement(std::make_shared<Instruction>(
        "cmp", tempVar + " = (" + leftTemp + " == " + rightTemp + ")"));
  } else if (operatorTemp == "!=") {
    current_parent->addElement(std::make_shared<Instruction>(
        "neq", tempVar + " = (" + leftTemp + " != " + rightTemp + ")"));
  } else if (operatorTemp == "<") {
    current_parent->addElement(std::make_shared<Instruction>(
        "lt", tempVar + " = (" + leftTemp + " < " + rightTemp + ")"));
  } else if (operatorTemp == ">") {
    current_parent->addElement(std::make_shared<Instruction>(
        "gt", tempVar + " = (" + leftTemp + " > " + rightTemp + ")"));
  } else if (operatorTemp == "<=") {
    current_parent->addElement(std::make_shared<Instruction>(
        "le", tempVar + " = (" + leftTemp + " <= " + rightTemp + ")"));
  } else if (operatorTemp == ">=") {
    current_parent->addElement(std::make_shared<Instruction>(
        "ge", tempVar + " = (" + leftTemp + " >= " + rightTemp + ")"));
  }
}

std::vector<std::string> Codegen::convertForCondition(ASTNode *node,
                                                      std::string conditionLabel, std::string bodyLabel,
                                                      std::string endLabel) {
  // Counter variable initialization
  std::string counterVarType = toLowerCase(node->getChildren()[0]->getValue());
  std::string counterVarName = node->getChildren()[1]->getValue();
  std::string counterVarValue =
      node->getChildren()[2]->getChildren()[0]->getValue();
  std::string counterVarTemporary = createTemporary();
  current_parent->addElement(std::make_shared<Instruction>(
      "alloc", counterVarTemporary + " = alloc " + counterVarType));
  current_parent->addElement(std::make_shared<Instruction>(
      "store", "store " + counterVarValue + ", " + counterVarTemporary));

  // Loop bound variable initialization
  std::string loopBoundVarName = "%" + node->getChildren()[5]->getValue();
  std::string loopBoundVarTemporary = createTemporary();
  current_parent->addElement(std::make_shared<Instruction>(
      "alloc", loopBoundVarTemporary + " = alloc int"));
  current_parent->addElement(std::make_shared<Instruction>(
      "store", "store " + loopBoundVarName + ", " + loopBoundVarTemporary));

  // For loop condition
  std::shared_ptr<Instruction> conditionLabelIR =
      std::make_shared<Instruction>("label", conditionLabel);
  current_parent->addElement(conditionLabelIR);
  switchParent(conditionLabelIR);

  std::string conditionCounterTemporary = createTemporary();
  current_parent->addElement(std::make_shared<Instruction>(
      "load", conditionCounterTemporary + " = load " + counterVarTemporary));

  std::string conditionLoopBoundTemporary = createTemporary();
  current_parent->addElement(std::make_shared<Instruction>(
      "load",
      conditionLoopBoundTemporary + " = load " + loopBoundVarTemporary));

  std::string conditionCompareTemporary = createTemporary();
  current_parent->addElement(std::make_shared<Instruction>(
      "lt", conditionCompareTemporary + " = (" + conditionCounterTemporary +
                " < " + conditionLoopBoundTemporary + ")"));

  // For loop condition break instruction
  current_parent->addElement(std::make_shared<Instruction>(
      "br", "br " + conditionCompareTemporary + ", label " + bodyLabel +
                ", label " + endLabel));
  popParent();

  return {conditionCounterTemporary, counterVarTemporary};
}

void Codegen::switchParent(std::shared_ptr<Instruction> newParent) {
  last_parent = current_parent;
  current_parent = newParent;
}

void Codegen::popParent() {
  current_parent = last_parent;
  last_parent = nullptr;
}

std::shared_ptr<Instruction>
Codegen::findInstruction(std::shared_ptr<CodegenElement> root,
                         std::shared_ptr<Instruction> instr) {
  auto instructionPtr = std::dynamic_pointer_cast<Instruction>(root);
  if (instructionPtr != nullptr) {
    for (const auto &child : instructionPtr->children) {
      if (child == instr) {
        return std::dynamic_pointer_cast<Instruction>(child);
      }

      if (auto found = findInstruction(child, instr)) {
        return found;
      }
    }
  }

  return nullptr;
}

std::string Codegen::toLowerCase(std::string string) {
  std::string result = string;
  std::transform(result.begin(), result.end(), result.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  return result;
}
