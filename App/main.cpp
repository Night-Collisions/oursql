#include <iostream>
#include "parser.cpp"
#include "Logic/Table.h"

int main() {
    //yydebug = 0;
    int flag;
    //yyin = fopen(argv[1], "r");
    yyin = stdin;
    flag = yyparse();
    //fclose(yyin);

    // yyout = fopen("dump.out", "w");
    // fclose(yyout);

    printf("%d\n", flag);

/*    double a = 100.123;
    Field f("name", Types::INT_, 1.12);

    f.getVal(&a);

    std::cout << a;*/

    return 0;
}
