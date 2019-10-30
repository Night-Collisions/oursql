#ifndef OURSQL_SERVER_LOGIC_RELATIONALOPERATIONS_INTERSECT_H_
#define OURSQL_SERVER_LOGIC_RELATIONALOPERATIONS_INTERSECT_H_

#include "../../../Engine/Table.h"
class Intersect {
   public:
    static Table makeIntersect(const Table& table1, const Table& table2,
                           std::unique_ptr<exc::Exception>& e);
};

#endif  // OURSQL_SERVER_LOGIC_RELATIONALOPERATIONS_INTERSECT_H_
