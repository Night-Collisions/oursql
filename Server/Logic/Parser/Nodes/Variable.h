#ifndef OURSQL_APP_LOGIC_PARSER_NODES_VARIABLE_H_
#define OURSQL_APP_LOGIC_PARSER_NODES_VARIABLE_H_

#include <string>

#include "../../../Engine/Column.h"
#include "Node.h"

class Variable : public Node {
   public:
    Variable(const std::string& name, DataType type)
        : Node(NodeType::variable), name_(name), data_type_(type) {}

    Variable(const std::string& name, DataType type,
             const std::vector<ColumnConstraint>& constraints)
        : Node(NodeType::variable),
          name_(name),
          data_type_(type),
          constraints_(constraints) {}

    void addVarcharLen(size_t len) { varchar_len_ = len; }
    size_t getVarcharLen() { return varchar_len_; }

    std::string getName() override { return name_; }
    DataType getType() { return data_type_; }
    std::vector<ColumnConstraint> getConstraints() { return constraints_; }

   private:
    std::string name_;
    DataType data_type_;
    std::vector<ColumnConstraint> constraints_;
    size_t varchar_len_;
};

#endif  // OURSQL_APP_LOGIC_PARSER_NODES_VARIABLE_H_
