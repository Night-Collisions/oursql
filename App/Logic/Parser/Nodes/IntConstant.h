//
// Created by Victor on 07.10.2019.
//

#ifndef OURSQL_APP_LOGIC_PARSER_NODES_INTCONSTANT_H_
#define OURSQL_APP_LOGIC_PARSER_NODES_INTCONSTANT_H_

#include "../../../Core/DataType.h"
#include "Constant.h"
#include "Node.h"

class IntConstant : public Constant {
   public:
    explicit IntConstant(const std::string& value) : Constant(value) {}

    DataType getDataType() override { return DataType::integer; }
};

#endif  // OURSQL_APP_LOGIC_PARSER_NODES_INTCONSTANT_H_
