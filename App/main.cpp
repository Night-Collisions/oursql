#include <iostream>

#include "Logic/Table.h"
#include "parser.cpp"

int main() {
    int flag;
    yydebug = 1;
    yyin = stdin;
    flag = yyparse();
    printf("%d\n", flag);
    return 0;
}
