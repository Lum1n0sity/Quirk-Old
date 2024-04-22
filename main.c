#include <stdio.h>
#include "lex.h"
#include "parser.h"

int main() {
    yyparse();
    return 0;
}