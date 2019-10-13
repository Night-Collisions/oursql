//
// Created by Victor on 13.10.2019.
//

#ifndef OURSQL_APP_LOGIC_PARSER_NODES_NULLCONSTANT_H_
#define OURSQL_APP_LOGIC_PARSER_NODES_NULLCONSTANT_H_

#include "Constant.h"
class NullConstant : public Constant {
   public:
    explicit NullConstant() : Constant("null") {}
    std::string getName() override { return std::string(); }

    DataType getDataType() override { return DataType::null_; }
};

#endif  // OURSQL_APP_LOGIC_PARSER_NODES_NULLCONSTANT_H_
