#include <iostream>

// TODO: вынести все их мэйн в отдельную функцию int run()

#include "parser.cpp"

int main() {
    int flag;
    yydebug = 0;
    yyin = stdin;
    yyout = stdout;
  //  setLogging(1);
  std::string a;
  std::cin >> a;
    parse_string(a.c_str());
    std::cout << "TEST TEST @@ " << buffer;
    std::cout << flag << std::endl;
    return 0;
}
