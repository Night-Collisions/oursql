#ifndef OURSQL_APP_LOGIC_FIELD_H_
#define OURSQL_APP_LOGIC_FIELD_H_

#include <string>
#include <utility>
#include <vector>

enum class DataType : unsigned int { integer, real, text, Count };

enum class FieldConstraint : unsigned int {
    null,
    primary_key,
    foreign_key,
    not_null,
    unique
};

class Field {
   public:
    Field(std::string name, const DataType type,
          const FieldConstraint constraint = FieldConstraint::null)
        : name_(std::move(name)), type_(type), constraint_(constraint) {}

    [[nodiscard]] DataType getType() const { return type_; };
    [[nodiscard]] std::string getName() const { return name_; };
    [[nodiscard]] FieldConstraint getConstraint() const { return constraint_; };
    [[nodiscard]] std::vector<std::string> getData() const { return data_; };

    void addData(const std::string&);

   private:
    std::string name_;
    DataType type_;
    FieldConstraint constraint_;
    std::vector<std::string> data_;
};

bool checkDataForType(const DataType type, const std::string& data);

#endif