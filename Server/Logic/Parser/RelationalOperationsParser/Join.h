#ifndef OURSQL_SERVER_LOGIC_RELATIONALOPERATIONS_JOIN_H_
#define OURSQL_SERVER_LOGIC_RELATIONALOPERATIONS_JOIN_H_

#include "../../../Engine/Table.h"
#include "../Nodes/RelExpr.h"

class Join {
   public:
    static Table makeJoin(const Table& table1, const Table& table2,
                          Expression* on_expr, std::unique_ptr<exc::Exception>& e);

    static Table makeInnerJoin(RelExpr* root,
                               std::unique_ptr<exc::Exception>& e);

    static Table makeOuterJoin(RelExpr* root,
                               std::unique_ptr<exc::Exception>& e);

    static Table makeCrossJoin(RelExpr* root,
                               std::unique_ptr<exc::Exception>& e);

    static Table makeFullJoin(RelExpr* root,
                              std::unique_ptr<exc::Exception>& e);

    static Table makeLeftJoin(RelExpr* root,
                              std::unique_ptr<exc::Exception>& e);

    static Table makeRightJoin(RelExpr* root,
                               std::unique_ptr<exc::Exception>& e);
};

#endif  // OURSQL_SERVER_LOGIC_RELATIONALOPERATIONS_JOIN_H_
