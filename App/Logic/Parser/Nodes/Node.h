
#ifndef OURSQL_APP_LOGIC_PARSER_NODES_NODE_H_
#define OURSQL_APP_LOGIC_PARSER_NODES_NODE_H_

enum class NodeType {
    node,
    query,  // root
    command,
    varList,
    variable,
    id,
    constraintList,
    selectList,
    condition,
    identList,
    constant,
    relation
};

class Node {
   public:
    explicit Node(NodeType type) : nodeType_(type) {}
    virtual ~Node() = default;

    virtual NodeType getNodeType() const { return nodeType_; }

   private:
    NodeType nodeType_;
};

#endif  // OURSQL_APP_LOGIC_PARSER_NODES_NODE_H_
