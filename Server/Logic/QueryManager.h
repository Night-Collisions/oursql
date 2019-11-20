#ifndef OURSQL_APP_LOGIC_QUERYMANAGER_H_
#define OURSQL_APP_LOGIC_QUERYMANAGER_H_

#include <mutex>
#include <set>
#include "../Engine/Column.h"
#include "../Engine/Table.h"
#include "../Engine/Value.h"
#include "Parser/Nodes/Query.h"
#include "Parser/Nodes/RelExpr.h"
#include "Parser/Nodes/Transaction.h"

using t_ull = unsigned long long;

typedef Table (*rel_func)(const Table& table1, const Table& table2,
                          Expression* on_expr,
                          std::unique_ptr<exc::Exception>& e,
                          RelOperNodeType join_type);

class QueryManager {
   public:
    QueryManager() = delete;

    static void execute(const Query& query, t_ull transact_num,
                        std::unique_ptr<exc::Exception>& e, std::ostream& out);

   private:
    static void createTable(const Query& query, t_ull transact_num,
                            std::unique_ptr<exc::Exception>& e,
                            std::ostream& out);
    static void showCreateTable(const Query& query, t_ull transact_num,
                                std::unique_ptr<exc::Exception>& e,
                                std::ostream& out);
    static void dropTable(const Query& query, t_ull transact_num,
                          std::unique_ptr<exc::Exception>& e,
                          std::ostream& out);

    static void select(const Query& query, t_ull transact_num,
                       std::unique_ptr<exc::Exception>& e, std::ostream& out);

    static void insert(const Query& query, t_ull transact_num,
                       std::unique_ptr<exc::Exception>& e, std::ostream& out);

    static void update(const Query& query, t_ull transact_num,
                       std::unique_ptr<exc::Exception>& e, std::ostream& out);

    static void remove(const Query& query, t_ull transact_num,
                       std::unique_ptr<exc::Exception>& e, std::ostream& out);

    static Table resolveRelationalOperTree(RelExpr* root,
                                           std::unique_ptr<exc::Exception>& e);

    static Table getFilledTable(const std::string& name,
                                std::unique_ptr<exc::Exception>& e);

    static std::map<std::string, bool> locked_tables_;
};

#endif  // OURSQL_APP_LOGIC_QUERYMANAGER_H_
