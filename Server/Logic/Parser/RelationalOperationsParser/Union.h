#ifndef OURSQL_SERVER_LOGIC_RELATIONALOPERATIONS_UNION_H_
#define OURSQL_SERVER_LOGIC_RELATIONALOPERATIONS_UNION_H_

#include "../../../Engine/Table.h"
#include "../Nodes/RelExpr.h"
class Union {
   public:
    static Table makeUnion(const Table& table1, const Table& table2,
                           std::unique_ptr<exc::Exception>& e);
};

#endif  // OURSQL_SERVER_LOGIC_RELATIONALOPERATIONS_UNION_H_
