
#ifndef OURSQL_APP_LOGIC_PARSER_NODES_VARIABLE_H_
#define OURSQL_APP_LOGIC_PARSER_NODES_VARIABLE_H_

#include <string>

#include "../../../Engine/Field.h"
#include "Node.h"

class Variable : public Node {
   public:
    Variable(const std::string& name, DataType type)
        : Node(NodeType::variable), name_(name), dataType_(type) {}

    Variable(const std::string& name, DataType type,
             const std::vector<FieldConstraint>& constraints)
        : Node(NodeType::variable),
          name_(name),
          dataType_(type),
          constraints_(constraints) {}

    std::string getName() { return name_; }
    DataType getType() { return dataType_; }
    std::vector<FieldConstraint> getConstraints() { return constraints_; }

   private:
    std::string name_;
    DataType dataType_;
    std::vector<FieldConstraint> constraints_;
};

#endif  // OURSQL_APP_LOGIC_PARSER_NODES_VARIABLE_H_
