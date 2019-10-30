
#ifndef OURSQL_SERVER_LOGIC_PARSER_RELATIONALOPERATIONSPARSER_HELPER_H_
#define OURSQL_SERVER_LOGIC_PARSER_RELATIONALOPERATIONSPARSER_HELPER_H_

#include <string>
#include <vector>
#include "../../../Core/DataType.h"
#include "../../../Engine/Value.h"
class Helper {
   public:
    static std::string recordToConcat(const std::vector<Value>& r);
    static bool checkTypes(DataType a, DataType b);
};

#endif  // OURSQL_SERVER_LOGIC_PARSER_RELATIONALOPERATIONSPARSER_HELPER_H_
