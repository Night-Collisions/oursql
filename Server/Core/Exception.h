#ifndef OURSQL_EXCEPTIONS_H
#define OURSQL_EXCEPTIONS_H

#include <string>

#include "DataType.h"

namespace exc {
enum class ExceptionType : unsigned int {
    syntax = 1,
    out_of_memory,
    was_loaded,
    was_not_loaded,
    repeat_column_in_table,
    repeat_column,
    ambiguous_column_name,
    different_column_sizes,
    div_by_zero,
    set_data_type_mismatch = 601,
    data_type_mismatch,
    no_operation_for_type,
    data_type_oversize,
    compare_data_type_mismatch,
    access_table_nonexistent = 701,
    access_column_nonexistent,
    incompatible_constraints = 801,
    redundant_constraints,
    duplicated_primary_key_in_column,
    duplicated_unique,
    null_not_null,
    create_table_name_column = 1001,
    create_table_repeat_table_name,
    insert_constants_more_columns = 1101,
    column_datatype_mismatch_union,
    column_sizes_union,
    null_column_in_union,
    column_datatype_mismatch_intersect,
    column_sizes_intersect,
    null_column_in_intersect,
    serialize_access_error,
    repeat_begin_transact,
    no_uncommited_transact
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

class WasLoaded : public Exception {
   public:
    WasLoaded(const std::string& table_name)
        : Exception(ExceptionType::was_loaded, table_name + "was loaded!") {}
};

class WasNotLoaded : public Exception {
   public:
    WasNotLoaded(const std::string& table_name)
        : Exception(ExceptionType::was_not_loaded,
                    table_name + "was not loaded!") {}
};

class UnionException : public Exception {
   public:
    UnionException(const std::string& msg, ExceptionType type)
        : Exception(type, msg) {}
};

class IntersectException : public Exception {
   public:
    IntersectException(const std::string& msg, ExceptionType type)
        : Exception(type, msg) {}
};

class RepeatColumnNameInTable : public Exception {
   public:
    RepeatColumnNameInTable(const std::string& table_name,
                            const std::string& column_name)
        : Exception(ExceptionType::repeat_column_in_table,
                    "repeat column " + column_name + " in table " + table_name +
                        ".") {}
};

class RepeatColumn : public Exception {
   public:
    RepeatColumn(const std::string& column_name)
        : Exception(ExceptionType::repeat_column,
                    "repeat column " + column_name + ".") {}
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

class AmbiguousColumnName : public Exception {
   public:
    AmbiguousColumnName(const std::string& msg)
        : Exception(ExceptionType::ambiguous_column_name, msg) {}
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

class DuplicatedPrimaryKeyInColumn : public TableException {
   public:
    DuplicatedPrimaryKeyInColumn(const std::string& table_name,
                                 const std::string& column_name1,
                                 const std::string& column_name2)
        : TableException(table_name,
                         ExceptionType::duplicated_primary_key_in_column,
                         "primary key is used in the column " + column_name1 +
                             " and in " + column_name2 + ".") {}
};

class DuplicatedUnique : public TableException {
   public:
    DuplicatedUnique(const std::string& table_name,
                     const std::string& column_name, const std::string& value)
        : TableException(
              table_name, ExceptionType::duplicated_unique,
              value + " is not unique is in the column " + column_name + ".") {}
};

class NullNotNull : public TableException {
   public:
    NullNotNull(const std::string& table_name, const std::string& column_name)
        : TableException(table_name, ExceptionType::null_not_null,
                         column_name + " can't contain null values.") {}
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
                    "column " + column_name + " in table " + table_name +
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

namespace ins {
class ConstantsMoreColumns : public Exception {
   public:
    ConstantsMoreColumns()
        : Exception(ExceptionType::insert_constants_more_columns,
                    "the number of constants is more than columns.") {}
};
}  // namespace ins

class SetDataTypeMismatch : public Exception {
   public:
    SetDataTypeMismatch(const DataType type, const std::string& data)
        : Exception(ExceptionType::set_data_type_mismatch,
                    "value " + data + " is not compatible with data type " +
                        DataType2String(type) + ".") {}
};

class NoOperationForType : public Exception {
   public:
    NoOperationForType(const DataType type1, const std::string& oper,
                       const DataType type2)
        : Exception(ExceptionType::no_operation_for_type,
                    "no operation '" + oper + "' for " +
                        DataType2String(type1) + " and " +
                        DataType2String(type2) + ".") {}

    NoOperationForType(const std::string& oper, const DataType type2)
        : Exception(ExceptionType::no_operation_for_type,
                    "no operation '" + oper + "' for " +
                        DataType2String(type2) + ".") {}

    NoOperationForType()
        : Exception(ExceptionType::no_operation_for_type,
                    "no operation for these types.") {}
};

// TODO: переделать аргумены конструктора, и переделать то, как оно вызывается в
// коде
class DataTypeOversize : public Exception {
   public:
    DataTypeOversize()
        : Exception(ExceptionType::data_type_oversize,
                    "value is too large for the data type.") {}

    explicit DataTypeOversize(const std::string& s)
        : Exception(ExceptionType::data_type_oversize,
                    "value of '" + s + "' is too large for the data type.") {}
};

class CompareDataTypeMismatch : public Exception {
   public:
    CompareDataTypeMismatch(const DataType type1, const DataType type2)
        : Exception(ExceptionType::compare_data_type_mismatch,
                    "can't compare " + DataType2String(type1) + " and " +
                        DataType2String(type2) + ".") {}
};

class DivByZero : public Exception {
   public:
    DivByZero(const std::string& mess)
        : Exception(ExceptionType::div_by_zero,
                    "division by zero '" + mess + "'.") {}
};

namespace tr {
class SerializeAccessError : public Exception {
   public:
    SerializeAccessError()
        : Exception(
              ExceptionType::serialize_access_error,
              "Could not serialize access.") {}
};

class RepeatBeginTransact : public Exception {
   public:
    RepeatBeginTransact()
        : Exception(ExceptionType::repeat_begin_transact,
                    "Transaction is already being started.") {}
};

class NoUncommitedTransact : public Exception {
   public:
    NoUncommitedTransact()
        : Exception(ExceptionType::no_uncommited_transact,
                    "No uncomitted transaction.") {}
};
}  // namespace tr

};  // namespace exc

#endif  // OURSQL_EXCEPTIONS_H
