#ifndef OURSQL_EXCEPTIONS_H
#define OURSQL_EXCEPTIONS_H

#include <string>

#include "DataType.h"

namespace exc {
enum class ExceptionType : unsigned int {
    syntax = 1,
    out_of_memory,
    repeat_column_in_table,
    set_data_type_mismatch = 601,
    compare_data_type_mismatch,
    access_table_nonexistent = 701,
    access_column_nonexistent,
    incompatible_constraints = 801,
    redundant_constraints,
    duplicated_primary_key,
    create_table_name_column = 1001,
    create_table_repeat_table_name
};

class Exception {
   public:
    Exception(const ExceptionType type, const std::string& message)
        : type_(type), message_(message) {}

    unsigned int getNumber() const { return static_cast<unsigned int>(type_); }

    ExceptionType getType() const { return type_; }

    std::string getMessage() const {
        return getStarMessage() + ":\n " + message_;
    }

   protected:
    virtual std::string getStarMessage() const {
        return "~~Exception " + std::to_string(getNumber());
    };

    const ExceptionType type_;
    const std::string message_;
};

class SyntaxException : public Exception {
   public:
    SyntaxException() : Exception(ExceptionType::syntax, "wrong syntax!") {}
};

class OutOfMemory : public Exception {
   public:
    OutOfMemory() : Exception(ExceptionType::out_of_memory, "out of memory!") {}
};

class RepeatColumnName : public Exception {
   public:
    RepeatColumnName(const std::string& table_name,
                     const std::string& column_name)
        : Exception(ExceptionType::repeat_column_in_table,
                    "repeat column " + column_name + " in table " + table_name +
                        ".") {}
};

class TableException : public Exception {
   public:
    TableException(const std::string& table_name, const ExceptionType type,
                   const std::string& message)
        : Exception(type, message), table_name_(table_name) {}

    std::string getStarMessage() const {
        return Exception::getStarMessage() + " in table " + table_name_;
    }

   protected:
    const std::string table_name_;
};

namespace constr {
class IncompatibleConstraints : public Exception {
   public:
    IncompatibleConstraints(const std::string& column_name,
                            const std::string& column_constraint1,
                            const std::string& column_constraint2)
        : Exception(ExceptionType::incompatible_constraints,
                    column_constraint1 + " can't be used with " +
                        column_constraint2 + " in column " + column_name +
                        "."){};
};

class RedundantConstraints
    : public Exception {  // TODO: добавить в место где они повторяются
   public:
    RedundantConstraints(const std::string& column_name,
                         const std::string& column_constraint)
        : Exception(ExceptionType::redundant_constraints,
                    "duplicate of constraint " + column_constraint +
                        " in column " + column_name + ".") {}
};

class DuplicatedPrimaryKey : public TableException {
   public:
    DuplicatedPrimaryKey(const std::string& table_name,
                         const std::string& column_name1,
                         const std::string& column_name2)
        : TableException(table_name, ExceptionType::duplicated_primary_key,
                         "primary key is used in the column " + column_name1 +
                             " and in " + column_name2 + ".") {}
};
}  // namespace constr

namespace acc {
class TableNonexistent : public Exception {
   public:
    TableNonexistent(const std::string& table_name)
        : Exception(ExceptionType::access_table_nonexistent,
                    "table " + table_name + " nonexistent.") {}
};

class ColumnNonexistent : public Exception {
   public:
    ColumnNonexistent(const std::string& column_name,
                      const std::string& table_name)
        : Exception(ExceptionType::access_column_nonexistent,
                    "column " + column_name + "in table " + table_name +
                        " nonexistent.") {}
};
};  // namespace acc

namespace cr_table {
class CreateTableException : public TableException {
   public:
    CreateTableException(const std::string& table_name,
                         const ExceptionType type, const std::string& message)
        : TableException(table_name, type, message){};

    std::string getStarMessage() const {
        return Exception::getStarMessage() + " in create table " + table_name_;
    }
};

class RepeatTableName : public CreateTableException {
   public:
    RepeatTableName(const std::string& table_name)
        : CreateTableException(table_name,
                               ExceptionType::create_table_repeat_table_name,
                               "this table name is repeated!") {}
};

class CreateTableExceptionInColumn : public CreateTableException {
   public:
    CreateTableExceptionInColumn(const std::string& table_name,
                                 const std::string& column_name,
                                 const ExceptionType type,
                                 const std::string& message)
        : CreateTableException(table_name, type, message),
          column_name_(column_name) {}

    std::string getStarMessage() const {
        return CreateTableException::getStarMessage() + "in column " +
               column_name_;
    }

   protected:
    const std::string column_name_;
};

class ColumnName : public CreateTableExceptionInColumn {
   public:
    ColumnName(const std::string& table_name, const std::string& column_name)
        : CreateTableExceptionInColumn(table_name, column_name,
                                       ExceptionType::create_table_name_column,
                                       "wrong name of column!") {}
};
};  // namespace cr_table

class SetDataTypeMismatch : public Exception {
   public:
    SetDataTypeMismatch(const DataType type, const std::string& data)
        : Exception(ExceptionType::set_data_type_mismatch,
                    "value " + data + " is not compatible with data type " +
                        DataType2String(type) + ".") {}
};

class CompareDataTypeMismatch : public Exception {
   public:
    CompareDataTypeMismatch(const DataType type1, const DataType type2)
        : Exception(ExceptionType::compare_data_type_mismatch,
                    "can't compare " + DataType2String(type1) + " and " +
                        DataType2String(type2) + ".") {}
};
};  // namespace exc

#endif  // OURSQL_EXCEPTIONS_H
