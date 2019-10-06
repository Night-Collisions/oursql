#ifndef OURSQL_APP_LOGIC_QUERYMANAGER_H_
#define OURSQL_APP_LOGIC_QUERYMANAGER_H_

#include <set>
#include "../Engine/Field.h"
#include "Parser/Nodes/Query.h"

class QueryManager {
   public:
    QueryManager() = delete;

    static void execute(const Query& query);
    static void createTable(const Query& query);

   private:
    static void checkConstraints(const std::set<FieldConstraint>& constraint);
};

#endif  // OURSQL_APP_LOGIC_QUERYMANAGER_H_
