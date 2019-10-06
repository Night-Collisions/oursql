
#ifndef OURSQL_APP_LOGIC_PARSER_NODES_QUERY_H_
#define OURSQL_APP_LOGIC_PARSER_NODES_QUERY_H_

#include <vector>
#include "Node.h"

class Query : public Node {
   public:
    explicit Query(const std::vector<Node*>& children)
        : Node(NodeType::query), children_(children) {}

    ~Query() {
        for (auto& ch : children_) {
            delete ch;
        }
    }

    std::vector<Node*> getChildren() { return children_; }

   private:
    std::vector<Node*> children_;
};

#endif
