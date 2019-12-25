#ifndef OURSQL_APP_LOGIC_PARSER_NODES_INDEXNODE_H_
#define OURSQL_APP_LOGIC_PARSER_NODES_INDEXNODE_H_

#include <string>

#include "Node.h"

class IndexNode : public Node {
   public:
    explicit IndexNode(const std::string& table_name,
                       const std::string& col_name)
        : Node(NodeType::index),
          table_name_(table_name),
          column_name_(col_name) {}

    [[nodiscard]] std::string getName() override { return std::string(); }
    std::string getTableName() { return table_name_; }
    std::string getColumnName() { return column_name_; }

   private:
    std::string table_name_;
    std::string column_name_;
};

#endif  // OURSQL_APP_LOGIC_PARSER_NODES_INDEXNODE_H_
