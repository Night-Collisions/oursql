//
// Created by Victor on 07.10.2019.
//

#ifndef OURSQL_APP_LOGIC_PARSER_NODES_INTCONSTANT_H_
#define OURSQL_APP_LOGIC_PARSER_NODES_INTCONSTANT_H_

#include "Node.h"

class IntConstant : public Node {
   public:
    explicit IntConstant(int value) : Node(NodeType::constant), value_(value) {}

    int getValue() { return value_; }

   private:
    int value_;
};

#endif  // OURSQL_APP_LOGIC_PARSER_NODES_INTCONSTANT_H_
