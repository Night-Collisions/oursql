#include <iostream>

#include "parser.cpp"

int main() {
    int flag;
    yydebug = 1;
    yyin = stdin;
    flag = yyparse();
    std::cout << flag << std::endl;
    return 0;
}
