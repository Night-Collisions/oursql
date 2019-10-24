#ifndef OURSQL_APP_LOGIC_PARSER_NODES_IDENT_H_
#define OURSQL_APP_LOGIC_PARSER_NODES_IDENT_H_

#include <string>
#include "Node.h"

class Ident : public Node {
   public:
    explicit Ident(const std::string& name)
        : Node(NodeType::ident), name_(name) {}
    explicit Ident(const std::string& table_name, const std::string& name)
        : Node(NodeType::ident), table_name_(table_name), name_(name) {}

    [[nodiscard]] std::string getName() override { return name_; }
    std::string getTableName() { return table_name_; }

    void setName(const std::string& name) { name_ = name; }
    void setTableName(const std::string& name) { table_name_ = name; }

   private:
    std::string name_;
    std::string table_name_;
};

#endif  // OURSQL_APP_LOGIC_PARSER_NODES_IDENT_H_
