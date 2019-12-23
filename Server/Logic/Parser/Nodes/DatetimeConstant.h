#ifndef OURSQL_SERVER_LOGIC_PARSER_NODES_DATETIMECONSTANT_H_
#define OURSQL_SERVER_LOGIC_PARSER_NODES_DATETIMECONSTANT_H_

#include "Constant.h"
class DatetimeConstant : public Constant {
   public:
    explicit DatetimeConstant(const std::string& value) : Constant(value) {}
    DataType getDataType() override { return DataType::datetime; }
    std::string getName() override { return std::string(); }
};

#endif  // OURSQL_SERVER_LOGIC_PARSER_NODES_DATETIMECONSTANT_H_
