#ifndef OURSQL_APP_LOGIC_QUERYMANAGER_H_
#define OURSQL_APP_LOGIC_QUERYMANAGER_H_

#include <mutex>
#include <set>

#include "../Engine/Column.h"
#include "../Engine/Table.h"
#include "../Engine/Value.h"
#include "Parser/Nodes/Query.h"
#include "Parser/Nodes/RelExpr.h"
#include "Parser/Nodes/SysTime.h"
#include "Parser/Nodes/Transaction.h"

using t_ull = unsigned long long;

typedef Table (*rel_func)(const Table& table1, const Table& table2,
                          Expression* on_expr,
                          std::unique_ptr<exc::Exception>& e,
                          RelOperNodeType join_type);

class QueryManager {
   public:
    QueryManager() = delete;

    static void execute(
        const Query& query, t_ull transact_num,
        std::unique_ptr<exc::Exception>& e, std::ostream& out,
        std::map<unsigned long long, std::set<std::string>>& locked_tables);

   private:
    static void createTable(const Query& query, t_ull transact_num,
                            std::unique_ptr<exc::Exception>& e,
                            std::ostream& out);
    static std::string getHistoryName(const std::string& name);
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

    static Table resolveRelationalOperTree(RelExpr* root, t_ull transact_num,
                                           std::unique_ptr<exc::Exception>& e);

    static Table getFilledTable(const std::string& name, t_ull transact_num,
                                std::unique_ptr<exc::Exception>& e);

    static Table getFilledTempTable(const std::string& name, t_ull transact_num,
                                    const SysTime& stime,
                                    std::unique_ptr<exc::Exception>& e);

    static void createTemporalTable(const std::string& name,
                                    const std::vector<Column>& columns,
                                    std::unique_ptr<exc::Exception>& e);

    static void insertTemporalTable(const std::string& name,
                                    const std::vector<Column>& cols,
                                    t_ull transact_num, const int sys_end_ind,
                                    std::vector<Value> rec,
                                    std::unique_ptr<exc::Exception>& e);
};

#endif  // OURSQL_APP_LOGIC_QUERYMANAGER_H_
