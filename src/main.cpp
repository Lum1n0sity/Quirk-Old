#include <iostream>
#include <string>
#include <fstream>
#include <regex>
#include <optional>
#include "parser.cpp"
#include "codegen.cpp"

int main() {
    Parser parser("test.qk");

    parser.Initalize();

    std::cout << "Initalize" << std::endl;

    ASTNode* root = parser.parse();

    printAST(root);

    CodeGen codegen;
    codegen.generate(root);
    codegen.printIR();

    return 0;
}
