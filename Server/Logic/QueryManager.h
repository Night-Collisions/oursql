#ifndef OURSQL_APP_LOGIC_QUERYMANAGER_H_
#define OURSQL_APP_LOGIC_QUERYMANAGER_H_

#include <set>
#include "../Engine/Column.h"
#include "../Engine/Table.h"
#include "../Engine/Value.h"
#include "Parser/Nodes/Query.h"
#include "Parser/Nodes/RelExpr.h"

typedef Table (*rel_func)(const Table& table1, const Table& table2,
                          Expression* on_expr,
                          std::unique_ptr<exc::Exception>& e);

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

    static Table resolveRelationalOperTree(RelExpr* root,
                                           std::unique_ptr<exc::Exception>& e);

    static Table getFilledTable(const std::string& name,
                                std::unique_ptr<exc::Exception>& e);

    static std::array<rel_func,
                      static_cast<unsigned int>(RelOperNodeType::Count)>
        relational_oper_;
};

#endif  // OURSQL_APP_LOGIC_QUERYMANAGER_H_
