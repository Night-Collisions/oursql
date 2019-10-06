#include <iostream>

#include "Logic/Parser/ParserManager.h"

int main() {

    ParserManager pm;
    auto a = pm.getParseTree(stdin);

    return 0;
}
