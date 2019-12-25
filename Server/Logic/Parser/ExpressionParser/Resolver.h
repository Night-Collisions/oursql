

#ifndef OURSQL_APP_LOGIC_PARSER_EXPRESSIONPARSER_RESOLVER_H_
#define OURSQL_APP_LOGIC_PARSER_EXPRESSIONPARSER_RESOLVER_H_

#include <map>

#include "../../../Engine/Column.h"
#include "../Nodes/Expression.h"

typedef std::map<std::string, std::map<std::string, Column>> t_column_infos;
typedef std::map<std::string, std::map<std::string, std::string>>
    t_record_infos;

typedef void (*func)(Expression* root, t_record_infos record,
                     std::unique_ptr<exc::Exception>& e);

enum class CompareCondition : unsigned int {
    assign,
    compare,
    operation,
    Count
};

class Resolver {
   public:
    static bool isGoodForIndex(const std::string& table_name, Expression* root,
                               std::map<std::string, int>& col_ind);

    static std::map<std::string, std::string> getRecordMap(
        const std::vector<Column>& cols, std::vector<Value> record,
        std::unique_ptr<exc::Exception>& e);

    static std::string resolve(const std::string& table1,
                               const std::string& table2,
                               t_column_infos column_infos, Expression* root,
                               t_record_infos record,
                               std::unique_ptr<exc::Exception>& e);

    static bool compareTypes(const std::string& table1,
                             const std::string& table2,
                             t_column_infos& column_info, Node* left,
                             Node* right, std::unique_ptr<exc::Exception>& e,
                             const CompareCondition& cond,
                             const std::string& oper);

   private:
    static void calculate(Expression* root, t_record_infos record,
                          std::unique_ptr<exc::Exception>& e);

    static void equal(Expression* root, t_record_infos record,
                      std::unique_ptr<exc::Exception>& e);
    static void notEqual(Expression* root, t_record_infos record,
                         std::unique_ptr<exc::Exception>& e);
    static void greater(Expression* root, t_record_infos record,
                        std::unique_ptr<exc::Exception>& e);
    static void greaterEqual(Expression* root, t_record_infos record,
                             std::unique_ptr<exc::Exception>& e);
    static void less(Expression* root, t_record_infos record,
                     std::unique_ptr<exc::Exception>& e);
    static void lessEqual(Expression* root, t_record_infos record,
                          std::unique_ptr<exc::Exception>& e);

    static void logicAnd(Expression* root, t_record_infos record,
                         std::unique_ptr<exc::Exception>& e);
    static void logicOr(Expression* root, t_record_infos record,
                        std::unique_ptr<exc::Exception>& e);
    static void logicNot(Expression* root, t_record_infos record,
                         std::unique_ptr<exc::Exception>& e);

    static void mul(Expression* root, t_record_infos record,
                    std::unique_ptr<exc::Exception>& e);
    static void div(Expression* root, t_record_infos record,
                    std::unique_ptr<exc::Exception>& e);
    static void add(Expression* root, t_record_infos record,
                    std::unique_ptr<exc::Exception>& e);
    static void sub(Expression* root, t_record_infos record,
                    std::unique_ptr<exc::Exception>& e);

    static void setStringValue(Expression* root, t_record_infos record,
                               std::unique_ptr<exc::Exception>& e,
                               std::string& a, std::string& b,
                               const CompareCondition& cond,
                               const std::string& oper);

    static void setDataTypes(Node* left, Node* right, DataType& a, DataType& b,
                             const std::string& table1,
                             const std::string& table2,
                             t_column_infos column_infos,
                             std::unique_ptr<exc::Exception>& e);

    static void setDataType(Node* nod, DataType& a, const std::string& table,
                            std::map<std::string, Column> column_info,
                            std::unique_ptr<exc::Exception>& e);

    static void bindColumnToTable(Node* nod,
                                  std::unique_ptr<exc::Exception>& e);

    static std::array<func, static_cast<unsigned int>(ExprUnit::Count)>
        operations_;

    static std::string table1_;
    static std::string table2_;
    static t_column_infos column_infos_;
};

#endif  // OURSQL_APP_LOGIC_PARSER_EXPRESSIONPARSER_RESOLVER_H_
