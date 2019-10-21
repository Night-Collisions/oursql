

#ifndef OURSQL_APP_LOGIC_PARSER_EXPRESSIONPARSER_RESOLVER_H_
#define OURSQL_APP_LOGIC_PARSER_EXPRESSIONPARSER_RESOLVER_H_

#include <rapidjson/document.h>
#include <map>
#include "../../../Engine/Column.h"
#include "../Nodes/Expression.h"

typedef void (*func)(Expression* root,
                     std::map<std::string, std::string> record,
                     std::unique_ptr<exc::Exception>& e);

enum class CompareCondition : unsigned int {
    assign,
    compare,
    operation,
    Count
};

class Resolver {
   public:
    static std::string resolve(const std::string& table,
                               std::map<std::string, Column> all_columns,
                               Expression* root,
                               std::map<std::string, std::string> record,
                               std::unique_ptr<exc::Exception>& e);

    static bool compareTypes(const std::string& table_name,
                             std::map<std::string, Column>& all_columns,
                             Node* left, Node* right,
                             std::unique_ptr<exc::Exception>& e,
                             const CompareCondition& cond,
                             const std::string& oper);

   private:
    static void calculate(Expression* root,
                          std::map<std::string, std::string> record,
                          std::unique_ptr<exc::Exception>& e);

    static void equal(Expression* root,
                      std::map<std::string, std::string> record,
                      std::unique_ptr<exc::Exception>& e);
    static void notEqual(Expression* root,
                         std::map<std::string, std::string> record,
                         std::unique_ptr<exc::Exception>& e);
    static void greater(Expression* root,
                        std::map<std::string, std::string> record,
                        std::unique_ptr<exc::Exception>& e);
    static void greaterEqual(Expression* root,
                             std::map<std::string, std::string> record,
                             std::unique_ptr<exc::Exception>& e);
    static void less(Expression* root,
                     std::map<std::string, std::string> record,
                     std::unique_ptr<exc::Exception>& e);
    static void lessEqual(Expression* root,
                          std::map<std::string, std::string> record,
                          std::unique_ptr<exc::Exception>& e);

    static void logicAnd(Expression* root,
                         std::map<std::string, std::string> record,
                         std::unique_ptr<exc::Exception>& e);
    static void logicOr(Expression* root,
                        std::map<std::string, std::string> record,
                        std::unique_ptr<exc::Exception>& e);
    static void logicNot(Expression* root,
                         std::map<std::string, std::string> record,
                         std::unique_ptr<exc::Exception>& e);

    static void mul(Expression* root, std::map<std::string, std::string> record,
                    std::unique_ptr<exc::Exception>& e);
    static void div(Expression* root, std::map<std::string, std::string> record,
                    std::unique_ptr<exc::Exception>& e);
    static void add(Expression* root, std::map<std::string, std::string> record,
                    std::unique_ptr<exc::Exception>& e);
    static void sub(Expression* root, std::map<std::string, std::string> record,
                    std::unique_ptr<exc::Exception>& e);

    static void setStringValue(Expression* root,
                               std::map<std::string, std::string> record,
                               std::unique_ptr<exc::Exception>& e,
                               std::string& a, std::string& b,
                               const CompareCondition& cond,
                               const std::string& oper);

    static void setDataTypes(Node* left, Node* right, DataType& a, DataType& b,
                             const std::string& table,
                             std::map<std::string, Column> all_columns,
                             std::unique_ptr<exc::Exception>& e);

    static void setDataType(Node* nod, DataType& a, const std::string& table,
                            std::map<std::string, Column> all_columns,
                            std::unique_ptr<exc::Exception>& e);

    static std::array<func, static_cast<unsigned int>(ExprUnit::Count)>
        operations_;

    static std::string table_;
    static std::map<std::string, Column> all_columns_;
};

#endif  // OURSQL_APP_LOGIC_PARSER_EXPRESSIONPARSER_RESOLVER_H_
