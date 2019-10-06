#ifndef OURSQL_APP_LOGIC_PARSER_PARSERMANAGER_H_
#define OURSQL_APP_LOGIC_PARSER_PARSERMANAGER_H_

#include <cstdio>
#include <iostream>
#include "Nodes/Query.h"

class ParserManager {
   public:
    Query* getParseTree(const std::string& query);
    Query* getParseTree(FILE* input = stdin);

    void setDebug(bool d) { debug_ = d; }

   private:
    int debug_{};
};

#endif  // OURSQL_APP_LOGIC_PARSER_PARSERMANAGER_H_
