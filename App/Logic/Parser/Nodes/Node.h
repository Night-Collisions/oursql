
#ifndef OURSQL_APP_LOGIC_PARSER_NODES_NODE_H_
#define OURSQL_APP_LOGIC_PARSER_NODES_NODE_H_

enum class NodeType {
    node,
    query, //root
    command,
    varList,
    variable,
    id,
    constraintList,
};

class Node {
   public:
    explicit Node(NodeType type) : nodeType_(type) {}
    virtual ~Node() = default;

    virtual NodeType getNodeType() { return nodeType_; }

   private:
    NodeType nodeType_;
};

#endif  // OURSQL_APP_LOGIC_PARSER_NODES_NODE_H_
