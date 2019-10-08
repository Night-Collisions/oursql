#ifndef OURSQL_APP_LOGIC_QUERYMANAGER_H_
#define OURSQL_APP_LOGIC_QUERYMANAGER_H_

#include <set>
#include "../Engine/Column.h"
#include "Parser/Nodes/Query.h"

class QueryManager {
   public:
    QueryManager() = delete;

    static void execute(const Query& query, std::unique_ptr<exc::Exception>& e);

   private:
    static void createTable(const Query& query, std::unique_ptr<exc::Exception>& e);
    static void showCreateTable(const Query& query, std::unique_ptr<exc::Exception>& e);
    static void dropTable(const Query& query, std::unique_ptr<exc::Exception>& e);

    //static void checkConstraints(const std::set<ColumnConstraint>& constraint);
};

#endif  // OURSQL_APP_LOGIC_QUERYMANAGER_H_
