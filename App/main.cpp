#include <iostream>

// TODO: вынести все их мэйн в отдельную функцию int run()

#include "parser.cpp"

int main() {
    int flag;
    yydebug = 0;
    yyin = stdin;
    yyout = stdout;
  //  setLogging(1);
    flag = yyparse();
    char resp[100];
    std::cout << flag << std::endl;
    return 0;
}
