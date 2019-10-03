#include <iostream>

// TODO: вынести все их мэйн в отдельную функцию int run()

#include "parser.cpp"

int main() {
    int flag;
    yydebug = 0;
    yyin = stdin;
    yyout = stdout;
    //  setLogging(1);
    yyparse();

    return 0;
}
