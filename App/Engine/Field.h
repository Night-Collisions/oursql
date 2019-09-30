#ifndef OURSQL_APP_LOGIC_FIELD_H_
#define OURSQL_APP_LOGIC_FIELD_H_

#include <string>
#include <utility>

enum class DataType : unsigned int { integer, real, text };

enum class FieldConstraint : unsigned int {
    null,
    primary_key,
    foreign_key,
    not_null,
    unique
};

class Field {
   public:
    Field(std::string name, const DataType type, std::string value = "",
          const FieldConstraint constraint = FieldConstraint::null)
        : name_(std::move(name)),
          type_(type),
          val_(std::move(value)),
          constraint_(constraint) {}

    [[nodiscard]] std::string getVal() const { return val_; };
    [[nodiscard]] DataType getType() const { return type_; };
    [[nodiscard]] std::string getName() const { return name_; };
    [[nodiscard]] FieldConstraint getConstraint() const { return constraint_; };

   private:
    std::string name_;
    DataType type_;
    std::string val_;
    FieldConstraint constraint_;
};

#endif