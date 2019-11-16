#ifndef OURSQL_APP_LOGIC_PARSER_PARSERMANAGER_H_
#define OURSQL_APP_LOGIC_PARSER_PARSERMANAGER_H_

#include <cstdio>
#include <memory>
#include <iostream>

#include "../../Core/Exception.h"
#include "Nodes/Query.h"
#include "Nodes/Transaction.h"

class ParserManager {
   public:
    Transaction* getParseTree(const std::string& query, std::unique_ptr<exc::Exception>& exception);

    void setDebug(bool d) { debug_ = d; }

   private:
    int debug_{};
};

#endif  // OURSQL_APP_LOGIC_PARSER_PARSERMANAGER_H_
