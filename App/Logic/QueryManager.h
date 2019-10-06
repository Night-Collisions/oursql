#ifndef OURSQL_APP_LOGIC_QUERYMANAGER_H_
#define OURSQL_APP_LOGIC_QUERYMANAGER_H_

#include "Parser/Nodes/Query.h"
class QueryManager {
   public:
    QueryManager() = delete;

    static void execute(const Query& query);
    static void createTable(const Query& query);

};

#endif  // OURSQL_APP_LOGIC_QUERYMANAGER_H_
