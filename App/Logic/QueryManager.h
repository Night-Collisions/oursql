#ifndef OURSQL_APP_LOGIC_QUERYMANAGER_H_
#define OURSQL_APP_LOGIC_QUERYMANAGER_H_

#include <set>
#include "../Engine/Column.h"
#include "Parser/Nodes/Query.h"

class QueryManager {
   public:
    QueryManager() = delete;

    static void execute(const Query& query);

   private:
    static void createTable(const Query& query);
    static void showCreateTable(const Query& query);
    static void dropTable(const Query& query);

    static void checkConstraints(const std::set<ColumnConstraint>& constraint);
};

#endif  // OURSQL_APP_LOGIC_QUERYMANAGER_H_
