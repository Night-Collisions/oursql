#include <iostream>

#include "Logic/Parser/ParserManager.h"
#include "Logic/QueryManager.h"

int main() {
    ParserManager pm;

    // auto a = pm.getParseTree();  // read from stdin by default
    std::unique_ptr<exc::Exception> e;

    auto a = pm.getParseTree(stdin, e);  // or give a query string

    QueryManager::execute(*a, e);
    delete a;  // это тоже работает

    return 0;
}
