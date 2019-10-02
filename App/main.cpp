#include <iostream>

// TODO: вынести все их мэйн в отдельную функцию int run()

#include "parser.cpp"

int main() {
    int flag;
    yydebug = 0;
    yyin = stdin;
    flag = yyparse();
    std::cout << flag << std::endl;
    return 0;
}
