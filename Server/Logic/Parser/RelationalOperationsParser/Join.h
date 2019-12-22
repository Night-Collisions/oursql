#ifndef OURSQL_SERVER_LOGIC_RELATIONALOPERATIONS_JOIN_H_
#define OURSQL_SERVER_LOGIC_RELATIONALOPERATIONS_JOIN_H_

#include <map>

#include "../../../Engine/Table.h"
#include "../Nodes/RelExpr.h"

class Join {
   public:
    static Table makeJoin(const Table& table1, const Table& table2,
                          Expression* on_expr,
                          std::unique_ptr<exc::Exception>& e,
                          RelOperNodeType type = RelOperNodeType::join);

   private:
    static bool isHashJoinOk(
        const std::string& name1, const std::string& name2,
        std::map<std::string, std::map<std::string, Column>> column_info,
        Expression* expr, std::unique_ptr<exc::Exception>& e);

    static std::vector<Value> getNullRecord(const std::vector<Column>& cols);
    static void pushBackMeta(
        const std::string& table_name, const std::vector<Column>& cols,
        std::vector<std::string>& col_names, std::vector<DataType>& types,
        std::vector<int>& varchar_sizes,
        std::vector<std::set<ColumnConstraint>>& constraints,
        std::vector<PeriodState>& period_states);
    static void initTable(
        Table& table, const std::vector<std::string>& col_names,
        const std::vector<DataType>& types,
        const std::vector<int>& varchar_sizes,
        const std::vector<std::set<ColumnConstraint>>& constraints,
        std::vector<PeriodState>& period_states,
        std::unique_ptr<exc::Exception>& e);
    static void setPosOfTheCols(Table& to_run, Table& to_hash,
                                const std::string& col_of_run,
                                const std::string& col_of_hash, int& pos_of_run,
                                int& pos_of_hash);
};

#endif  // OURSQL_SERVER_LOGIC_RELATIONALOPERATIONS_JOIN_H_
