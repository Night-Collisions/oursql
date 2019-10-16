//
// Created by Victor on 07.10.2019.
//

#ifndef OURSQL_APP_LOGIC_PARSER_NODES_TEXTCONSTANT_H_
#define OURSQL_APP_LOGIC_PARSER_NODES_TEXTCONSTANT_H_

#include <string>
#include "../../../Core/DataType.h"
#include "Constant.h"
#include "Node.h"
class TextConstant : public Constant {
   public:
    explicit TextConstant(const std::string& value) : Constant(value) {}

    DataType getDataType() override { return DataType::varchar; }
};

#endif  // OURSQL_APP_LOGIC_PARSER_NODES_TEXTCONSTANT_H_
