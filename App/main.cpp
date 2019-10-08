#include <iostream>

#include "Logic/Parser/ParserManager.h"
#include "Logic/QueryManager.h"

#include <string>
#include <iostream>
#include <sstream>

int read_sum(std::istream& s) {
    int a, b;
    s >> a >> b;
    return a + b;
}

int main() {

    std::stringstream ss;
    ss << 100 << ' ' << 200;



    std::cout << read_sum(ss) << std::endl;


//    ParserManager pm;
//
//    // auto a = pm.getParseTree();  // read from stdin by default
//    std::unique_ptr<exc::Exception> e;
//
//    auto a = pm.getParseTree(stdin, e);  // or give a query string
//
//    QueryManager::execute(*a, e);
//    delete a;  // это тоже работает

    return 0;
}
