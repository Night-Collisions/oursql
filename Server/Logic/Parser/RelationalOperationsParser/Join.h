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
};

#endif  // OURSQL_SERVER_LOGIC_RELATIONALOPERATIONS_JOIN_H_
