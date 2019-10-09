#ifndef OURSQL_APP_LOGIC_QUERYMANAGER_H_
#define OURSQL_APP_LOGIC_QUERYMANAGER_H_

#include <set>
#include "../Engine/Column.h"
#include "Parser/Nodes/Query.h"

class QueryManager {
   public:
    QueryManager() = delete;

    static void execute(const Query& query, std::unique_ptr<exc::Exception>& e,
                        std::ostream& out);

   private:
    static void createTable(const Query& query,
                            std::unique_ptr<exc::Exception>& e, std::ostream& out);
    static void showCreateTable(const Query& query,
                                std::unique_ptr<exc::Exception>& e, std::ostream& out);
    static void dropTable(const Query& query,
                          std::unique_ptr<exc::Exception>& e, std::ostream& out);

    static void select(const Query& query,
    std::unique_ptr<exc::Exception>& e, std::ostream& out);
};

#endif  // OURSQL_APP_LOGIC_QUERYMANAGER_H_
