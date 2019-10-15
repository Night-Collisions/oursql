

#ifndef OURSQL_APP_LOGIC_PARSER_EXPRESSIONPARSER_RESOLVER_H_
#define OURSQL_APP_LOGIC_PARSER_EXPRESSIONPARSER_RESOLVER_H_

#include <rapidjson/document.h>
#include <map>
#include "../../../Engine/Column.h"
#include "../Nodes/Expression.h"

typedef void (*func)(Expression* root, std::unique_ptr<exc::Exception>& e);

class Resolver {
   public:
    static void resolve(Expression* root, const rapidjson::Value& record,
                        std::unique_ptr<exc::Exception>& e);

    static bool compareTypes(const std::string& table_name,
                             std::map<std::string, Column>& all_columns,
                             Node* left, Node* right,
                             std::unique_ptr<exc::Exception>& e, bool);

   private:
    void calculate(Expression* root, const rapidjson::Value& record,
                   std::unique_ptr<exc::Exception>& e);

    static void equal(Expression* root, std::unique_ptr<exc::Exception>& e);
    static void notEqual(Expression* root, std::unique_ptr<exc::Exception>& e);
    static void greater(Expression* root, std::unique_ptr<exc::Exception>& e);
    static void greaterEqual(Expression* root,
                             std::unique_ptr<exc::Exception>& e);
    static void less(Expression* root, std::unique_ptr<exc::Exception>& e);
    static void lessEqual(Expression* root, std::unique_ptr<exc::Exception>& e);

    static void logicAnd(Expression* root, std::unique_ptr<exc::Exception>& e);
    static void logicOr(Expression* root, std::unique_ptr<exc::Exception>& e);
    static void logicNot(Expression* root, std::unique_ptr<exc::Exception>& e);

    static void mul(Expression* root, std::unique_ptr<exc::Exception>& e);
    static void div(Expression* root, std::unique_ptr<exc::Exception>& e);
    static void add(Expression* root, std::unique_ptr<exc::Exception>& e);
    static void sub(Expression* root, std::unique_ptr<exc::Exception>& e);

    static void deductVal(Expression* root, std::unique_ptr<exc::Exception>& e);

    static std::map<unsigned int, func> operations_;
};

#endif  // OURSQL_APP_LOGIC_PARSER_EXPRESSIONPARSER_RESOLVER_H_
