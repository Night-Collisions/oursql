

#ifndef OURSQL_APP_LOGIC_PARSER_EXPRESSIONPARSER_RESOLVER_H_
#define OURSQL_APP_LOGIC_PARSER_EXPRESSIONPARSER_RESOLVER_H_

#include <rapidjson/document.h>
#include <map>
#include "../../../Engine/Column.h"
#include "../Nodes/Expression.h"

class Resolver {
   public:
    static void resolve(Expression* root, const rapidjson::Value& record);

    static bool compareTypes(const std::string& table_name,
                             std::map<std::string, Column>& all_columns,
                             Node* left, Node* right,
                             std::unique_ptr<exc::Exception>& e, bool);
};

#endif  // OURSQL_APP_LOGIC_PARSER_EXPRESSIONPARSER_RESOLVER_H_
