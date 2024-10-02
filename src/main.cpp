#include <iostream>
#include <string>
#include <fstream>
#include <regex>
#include <optional>

#include "parser.h"
#include "codegen.h"

int main() {
    Parser parser("test.qk");

    parser.Initalize();
 
    ASTNode* root = parser.parse();

    printAST(root, 0);
  
    // Codegen codegenInstance;
    // codegenInstance.ConvertAST(root);

    delete root;

    return 0;
}
