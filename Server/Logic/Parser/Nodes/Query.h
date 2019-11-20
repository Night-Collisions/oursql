#ifndef OURSQL_APP_LOGIC_PARSER_NODES_QUERY_H_
#define OURSQL_APP_LOGIC_PARSER_NODES_QUERY_H_

#include <map>
#include <vector>
#include "Query.h"
#include "Node.h"

enum class CommandType : unsigned int {
    unknown,
    create_table,
    show_create_table,
    drop_table,
    select,
    insert,
    update,
    remove,
    Count
};

class Query : public Node {
   public:
    explicit Query(const std::map<NodeType, Node*>& children, CommandType type)
        : Node(NodeType::query), children_(children), cmd_type_(type) {}

    ~Query() override {
        for (auto& ch : children_) {
            delete ch.second;
        }
    }

    [[nodiscard]] std::map<NodeType, Node*> getChildren() const {
        return children_;
    }

    [[nodiscard]] CommandType getCmdType() const { return cmd_type_; }

    std::string getName() override { return std::string(); }

   private:
    CommandType cmd_type_;
    std::map<NodeType, Node*> children_;
};

#endif
