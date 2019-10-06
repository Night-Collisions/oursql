#ifndef OURSQL_EXCEPTIONS_H
#define OURSQL_EXCEPTIONS_H

#include <string>

#include "../Engine/Field.h"

namespace exc {
enum class ExceptionType : unsigned int {
    data_type_mismatch,
    repeat_field_in_table,
    access_table_nonexistent = 701,
    access_field_nonexistent,
    incompatible_constraints = 801,
    redundant_constraints,
    duplicated_primary_key,
    create_table_name_table = 1001,
    create_table_name_field,
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

    operator bool() const { return type_ != ExceptionType::null; }

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

class RepeatFieldName : public ExceptionInCommand {
   public:
    RepeatFieldName(const std::string& command, const std::string& table_name,
                    const std::string& field_name)
        : ExceptionInCommand(
              command, ExceptionType::repeat_field_in_table,
              "repeat field " + field_name + " in table " + table_name + ".") {}
};

namespace constr {
class IncompatibleConstraints : public ExceptionInCommand {
   public:
    IncompatibleConstraints(const std::string& command,
                            const std::string& field_name,
                            const FieldConstraint& a, const FieldConstraint& b)
        : ExceptionInCommand(command, ExceptionType::incompatible_constraints,
                             FieldConstraint2String(a) +
                                 " can't be used with " +
                                 FieldConstraint2String(b) + " in field " +
                                 field_name + "."){};
};

class RedundantConstraints : public ExceptionInCommand {
   public:
    RedundantConstraints(const std::string& command,
                         const std::string& field_name,
                         const FieldConstraint& a)
        : ExceptionInCommand(command, ExceptionType::redundant_constraints,
                             "duplicate of constraint " +
                                 FieldConstraint2String(a) + " in field " +
                                 field_name + ".") {}
};

class DuplicatedPrimaryKey : public ExceptionInCommand {
   public:
    DuplicatedPrimaryKey(const std::string& command,
                         const std::string& field_name1,
                         const std::string& field_name2)
        : ExceptionInCommand(command, ExceptionType::duplicated_primary_key,
                             "primary key is used in the field " + field_name1 +
                                 " and in " + field_name2 + ".") {}
};
}  // namespace constr

namespace acc {
class TableNonexistent : public ExceptionInCommand {
   public:
    TableNonexistent(const std::string& table_name, const std::string& command)
        : ExceptionInCommand(command, ExceptionType::access_table_nonexistent,
                             "table " + table_name + " nonexistent.") {}
};

class FieldNonexistent : public ExceptionInCommand {
   public:
    FieldNonexistent(const std::string& field_name,
                     const std::string& table_name, const std::string& command)
        : ExceptionInCommand(command, ExceptionType::access_field_nonexistent,
                             "field " + field_name + "in table " + table_name +
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

class CreateTableExceptionInField : public CreateTableException {
   public:
    CreateTableExceptionInField(const std::string& table_name,
                                const std::string& field_name,
                                const ExceptionType type,
                                const std::string& message)
        : CreateTableException(table_name, type, message),
          field_name_(field_name) {}

    std::string getStarMessage() const {
        return CreateTableException::getStarMessage() + "in field " +
               field_name_;
    }

   protected:
    const std::string field_name_;
};

class FieldName : public CreateTableExceptionInField {
   public:
    FieldName(const std::string& table_name, const std::string& field_name)
        : CreateTableExceptionInField(table_name, field_name,
                                      ExceptionType::create_table_name_field,
                                      "wrong name of field!") {}
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
