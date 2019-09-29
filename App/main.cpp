#include <iostream>
#include "parser.cpp"

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
    return 0;
}
