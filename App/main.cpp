#include <iostream>

#include "Logic/Parser/ParserManager.h"

int main() {

    ParserManager pm;

    //auto a = pm.getParseTree();  // read from stdin by default
    exc::Exception* e;
    auto a = pm.getParseTree("show create table a;", e); //or give a query string

    delete a; // это тоже работает

    return 0;
}
