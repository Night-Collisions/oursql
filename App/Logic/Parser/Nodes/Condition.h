#ifndef OURSQL_APP_LOGIC_PARSER_NODES_CONDITION_H_
#define OURSQL_APP_LOGIC_PARSER_NODES_CONDITION_H_

#include "Node.h"

enum class Relation {
    equal,
    not_equal,
    greater,
    greater_eq,
    less,
    less_eq
};

class Condition : public Node {


};

#endif  // OURSQL_APP_LOGIC_PARSER_NODES_CONDITION_H_
