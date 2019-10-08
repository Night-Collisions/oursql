
#ifndef OURSQL_APP_LOGIC_PARSER_NODES_RELATION_H_
#define OURSQL_APP_LOGIC_PARSER_NODES_RELATION_H_

#include "Node.h"

enum class RelationType {
    equal = 0,
    not_equal,
    greater,
    greater_eq,
    less,
    less_eq,
    Count
};

class Relation : public Node {
   public:
    Relation(Node* left, RelationType type, Node* right)
        : Node(NodeType::relation), left_(left), relation_(type), right_(right) {}

    ~Relation() override {
        delete left_;
        delete right_;
    }

    Node* getLeft() { return left_; }
    Node* getRight() { return right_; }
    RelationType getRelation() { return relation_; }

   private:
    Node* left_;
    Node* right_;
    RelationType relation_;
};

#endif  // OURSQL_APP_LOGIC_PARSER_NODES_RELATION_H_
