#ifndef OURSQL_EXCEPTIONS_H
#define OURSQL_EXCEPTIONS_H

#include <string>

#include "../Engine/Column.h"

namespace exc {
enum class ExceptionType : unsigned int {
    data_type_mismatch,
    repeat_column_in_table,
    access_table_nonexistent = 701,
    access_column_nonexistent,
    incompatible_constraints = 801,
    redundant_constraints,
    duplicated_primary_key,
    create_table_name_table = 1001,
    create_table_name_column,
    create_table_repeat_table_name
};

class Exception {
   public:
    Exception(const ExceptionType type, const std::string& message)
        : type_(type), message_(message) {}

    unsigned int getNumber() const { return static_cast<unsigned int>(type_); }

    ExceptionType getType() const { return type_; }

    std::string getMessage() const {
        return getStarMessage() + ": " + message_;
    }

   protected:
    std::string getStarMessage() const {
        return "~~Error " + std::to_string(getNumber());
    };

    const ExceptionType type_;
    const std::string message_;
};

class ExceptionInCommand : public Exception {
   public:
    ExceptionInCommand(const std::string& command, const ExceptionType type,
                       const std::string& message)
        : Exception(type, message), command_(command) {}

   protected:
    std::string getStarMessage() const {
        return Exception::getStarMessage() + "in command \"" + command_ + "\"";
    };
    const std::string command_;
};

class RepeatColumnName : public ExceptionInCommand {
   public:
    RepeatColumnName(const std::string& command, const std::string& table_name,
                    const std::string& column_name)
        : ExceptionInCommand(
              command, ExceptionType::repeat_column_in_table,
              "repeat column " + column_name + " in table " + table_name + ".") {}
};

namespace constr {
class IncompatibleConstraints : public ExceptionInCommand {
   public:
    IncompatibleConstraints(const std::string& command,
                            const std::string& column_name,
                            const ColumnConstraint& a, const ColumnConstraint& b)
        : ExceptionInCommand(command, ExceptionType::incompatible_constraints,
                             ColumnConstraint2String(a) +
                                 " can't be used with " +
                                 ColumnConstraint2String(b) + " in column " +
                                 column_name + "."){};
};

class RedundantConstraints : public ExceptionInCommand {
   public:
    RedundantConstraints(const std::string& command,
                         const std::string& column_name,
                         const ColumnConstraint& a)
        : ExceptionInCommand(command, ExceptionType::redundant_constraints,
                             "duplicate of constraint " +
                                 ColumnConstraint2String(a) + " in column " +
                                 column_name + ".") {}
};

class DuplicatedPrimaryKey : public ExceptionInCommand {
   public:
    DuplicatedPrimaryKey(const std::string& command,
                         const std::string& column_name1,
                         const std::string& column_name2)
        : ExceptionInCommand(command, ExceptionType::duplicated_primary_key,
                             "primary key is used in the column " + column_name1 +
                                 " and in " + column_name2 + ".") {}
};
}  // namespace constr

namespace acc {
class TableNonexistent : public ExceptionInCommand {
   public:
    TableNonexistent(const std::string& table_name, const std::string& command)
        : ExceptionInCommand(command, ExceptionType::access_table_nonexistent,
                             "table " + table_name + " nonexistent.") {}
};

class ColumnNonexistent : public ExceptionInCommand {
   public:
    ColumnNonexistent(const std::string& column_name,
                     const std::string& table_name, const std::string& command)
        : ExceptionInCommand(command, ExceptionType::access_column_nonexistent,
                             "column " + column_name + "in table " + table_name +
                                 " nonexistent.") {}
};
};  // namespace acc

class TableException : public Exception {
   public:
    TableException(const std::string& table_name, const ExceptionType type,
                   const std::string& message)
        : Exception(type, message), table_name_(table_name) {}

    std::string getStarMessage() const {
        return Exception::getStarMessage() + "in table " + table_name_;
    }

   protected:
    const std::string table_name_;
};

namespace cr_table {
class CreateTableException : public TableException {
   public:
    CreateTableException(const std::string& table_name,
                         const ExceptionType type, const std::string& message)
        : TableException(table_name, type, message){};

    std::string getStarMessage() const {
        return Exception::getStarMessage() + "in create table " + table_name_;
    }
};

class TableName : public CreateTableException {
    TableName(const std::string& table_name)
        : CreateTableException(table_name,
                               ExceptionType::create_table_name_table,
                               "wrong name of table!") {}
};

class RepeatTableName : public CreateTableException {
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

class DataTypeMismatch : public Exception {
   public:
    DataTypeMismatch(const DataType type, const std::string& data)
        : Exception(ExceptionType::data_type_mismatch,
                    "value " + data + " is not compatible with data type " +
                        DataType2String(type) + ".") {}
};
};  // namespace exc

#endif  // OURSQL_EXCEPTIONS_H
