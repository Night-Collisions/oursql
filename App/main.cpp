#include <iostream>

// TODO: вынести все их мэйн в отдельную функцию int run()

#include "parser.cpp"

int main() {
    yydebug = 0;
    yyin = stdin;
    yyout = stdout;
    yyparse();
    return 0;
}
