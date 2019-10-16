
#ifndef OURSQL_APP_LOGIC_PARSER_NODES_NODE_H_
#define OURSQL_APP_LOGIC_PARSER_NODES_NODE_H_

#include <string>

enum class NodeType {
    node,
    query,  // root
    command,
    var_list,
    variable,
    ident,
    constraint_list,
    select_list,
    condition,
    ident_list,
    constant_list,
    constant,
    int_constant,
    real_constant,
    text_constant,
    relation
};

class Node {
   public:
    explicit Node(NodeType type) : nodeType_(type) {}
    virtual ~Node() = default;

    virtual NodeType getNodeType() const { return nodeType_; }

    virtual std::string getName() = 0;

   private:
    NodeType nodeType_;
};

#endif  // OURSQL_APP_LOGIC_PARSER_NODES_NODE_H_