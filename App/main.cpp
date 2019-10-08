#include <iostream>

#include "Logic/Parser/Nodes/Relation.h"
#include "Logic/Parser/ParserManager.h"
#include "Logic/QueryManager.h"

#include <iostream>
#include <sstream>
#include <string>

int read_sum(std::istream& s) {
    int a, b;
    s >> a >> b;
    return a + b;
}

int main() {

/*    std::stringstream ss;
    ss << 100 << ' ' << 200;



    std::cout << read_sum(ss) << std::endl;*/


    ParserManager pm;

    // auto a = pm.getParseTree();  // read from stdin by default
    std::unique_ptr<exc::Exception> e;

    auto a = pm.getParseTree("select *, A, employers.B from employers where 5 = id;", e);

    QueryManager::execute(*a, e);
    delete a;  // это тоже работает

    return 0;
}
