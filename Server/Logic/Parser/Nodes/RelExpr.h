#ifndef OURSQL_SERVER_LOGIC_PARSER_NODES_RELEXPR_H_
#define OURSQL_SERVER_LOGIC_PARSER_NODES_RELEXPR_H_

enum class RelOperNodeType : unsigned int {
    join,
    inner_join,
    outer_join,
    full_join,
    cross_join,
    left_join,
    right_join,
    union_,
    intersect,
    table_ident,
    Count
};

class RelExpr {};

#endif  // OURSQL_SERVER_LOGIC_PARSER_NODES_RELEXPR_H_
