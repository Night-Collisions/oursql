#ifndef OURSQL_APP_LOGIC_PARSER_PARSERMANAGER_H_
#define OURSQL_APP_LOGIC_PARSER_PARSERMANAGER_H_

#include <cstdio>
#include <memory>
#include <iostream>

#include "Nodes/Query.h"
#include "../../Core/Exception.h"

class ParserManager {
   public:
    Query* getParseTree(const std::string& query, std::unique_ptr<exc::Exception>& exception);
    Query* getParseTree(std::unique_ptr<exc::Exception>& exception);

    void setDebug(bool d) { debug_ = d; }

   private:
    int debug_{};
};

#endif  // OURSQL_APP_LOGIC_PARSER_PARSERMANAGER_H_
