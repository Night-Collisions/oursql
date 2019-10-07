//
// Created by Victor on 07.10.2019.
//

#ifndef OURSQL_APP_LOGIC_PARSER_NODES_TEXTCONSTANT_H_
#define OURSQL_APP_LOGIC_PARSER_NODES_TEXTCONSTANT_H_

#include <string>
#include "Node.h"
class TextConstant : public Node {
   public:
    explicit TextConstant(const std::string& value)
        : Node(NodeType::constant), value_(value) {}

    std::string getValue() { return value_; }

   private:
    std::string value_;
};

#endif  // OURSQL_APP_LOGIC_PARSER_NODES_TEXTCONSTANT_H_
