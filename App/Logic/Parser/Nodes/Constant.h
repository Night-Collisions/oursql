//
// Created by Victor on 10.10.2019.
//

#ifndef OURSQL_APP_LOGIC_PARSER_NODES_CONSTANT_H_
#define OURSQL_APP_LOGIC_PARSER_NODES_CONSTANT_H_

#include "../../../Core/DataType.h"
#include "Node.h"
class Constant : public Node {
   public:
    explicit Constant(const std::string& value)
        : Node(NodeType::int_constant), value_(value) {}

    virtual DataType getDataType() = 0;

    std::string getValue() { return value_; }

   protected:
    std::string value_;
};

#endif  // OURSQL_APP_LOGIC_PARSER_NODES_CONSTANT_H_