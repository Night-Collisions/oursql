#ifndef OURSQL_APP_LOGIC_QUERYMANAGER_H_
#define OURSQL_APP_LOGIC_QUERYMANAGER_H_

#include <set>
#include "../Engine/Column.h"
#include "../Engine/Table.h"
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

    static bool compareTypes(const std::string& table_name,
                             std::map<std::string, Column>& all_columns,
                             Node* left, Node* right,
                             std::unique_ptr<exc::Exception>& e, bool);

    static void setValue(Node* nod, std::string& value);

};

#endif  // OURSQL_APP_LOGIC_QUERYMANAGER_H_