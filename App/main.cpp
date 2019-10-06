#include <iostream>

#include "Logic/Parser/ParserManager.h"

int main() {

    ParserManager pm;

    //auto a = pm.getParseTree();  // read from stdin by default
    auto a = pm.getParseTree("show create table a;"); //or give a query string

    delete a; // это тоже работает

    return 0;
}
