#ifndef OURSQL_APP_LOGIC_QUERYMANAGER_H_
#define OURSQL_APP_LOGIC_QUERYMANAGER_H_

#include <set>
#include "../Engine/Column.h"
#include "../Engine/Table.h"
#include "../Engine/Value.h"
#include "Parser/Nodes/Query.h"

class QueryManager {
   public:
    QueryManager() = delete;

    static void execute(const Query& query, std::unique_ptr<exc::Exception>& e,
                        std::ostream& out);

   private:
    static void createTable(const Query& query,
                            std::unique_ptr<exc::Exception>& e,
                            std::ostream& out);
    static void showCreateTable(const Query& query,
                                std::unique_ptr<exc::Exception>& e,
                                std::ostream& out);
    static void dropTable(const Query& query,
                          std::unique_ptr<exc::Exception>& e,
                          std::ostream& out);

    static void select(const Query& query, std::unique_ptr<exc::Exception>& e,
                       std::ostream& out);

    static void insert(const Query& query, std::unique_ptr<exc::Exception>& e,
                       std::ostream& out);

    static void update(const Query& query, std::unique_ptr<exc::Exception>& e,
                       std::ostream& out);

    static void remove(const Query& query, std::unique_ptr<exc::Exception>& e,
                       std::ostream& out);

    static void setValue(Node* nod, std::string& value);

    static std::map<std::string, std::string> mapFromFetch(
        std::map<std::string, Column> all_columns, std::vector<Value> ftch);
};

#endif  // OURSQL_APP_LOGIC_QUERYMANAGER_H_
