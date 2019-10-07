//
// Created by Victor on 07.10.2019.
//

#ifndef OURSQL_APP_LOGIC_PARSER_NODES_REALCONSTANT_H_
#define OURSQL_APP_LOGIC_PARSER_NODES_REALCONSTANT_H_

#include "Node.h"
class RealConstant : public Node {
   public:
    explicit RealConstant(float value) : Node(NodeType::constant), value_(value) {}

    float getValue() { return value_; }

   private:
    float value_;
};
#endif  // OURSQL_APP_LOGIC_PARSER_NODES_REALCONSTANT_H_
