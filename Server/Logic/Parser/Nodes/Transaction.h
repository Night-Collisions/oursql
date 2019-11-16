#ifndef OURSQL_SERVER_LOGIC_PARSER_NODES_TRANSACTION_H_
#define OURSQL_SERVER_LOGIC_PARSER_NODES_TRANSACTION_H_

#include <vector>
#include "Node.h"
#include "Query.h"

class Transaction : public Node {
   public:
    explicit Transaction(const std::vector<Query*>& queries)
        : Node(NodeType::transaction) {}
    ~Transaction() override {
        for (auto& q : queries_) {
            delete q;
        }
        queries_.clear();
    }

    std::vector<Query*> getChildren() { return queries_; }
    std::string getName() override { return ""; }

   private:
    std::vector<Query*> queries_;
};

#endif  // OURSQL_SERVER_LOGIC_PARSER_NODES_TRANSACTION_H_
