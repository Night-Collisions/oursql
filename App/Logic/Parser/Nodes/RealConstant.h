//
// Created by Victor on 07.10.2019.
//

#ifndef OURSQL_APP_LOGIC_PARSER_NODES_REALCONSTANT_H_
#define OURSQL_APP_LOGIC_PARSER_NODES_REALCONSTANT_H_

#include "../../../Core/DataType.h"
#include "Constant.h"
#include "Node.h"

class RealConstant : public Constant {
   public:
    explicit RealConstant(const std::string& value) : Constant(value) {}

    DataType getDataType() override { return DataType::real; }
};
#endif  // OURSQL_APP_LOGIC_PARSER_NODES_REALCONSTANT_H_
