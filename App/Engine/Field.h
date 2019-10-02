#ifndef OURSQL_APP_LOGIC_FIELD_H_
#define OURSQL_APP_LOGIC_FIELD_H_

#include <map>
#include <set>
#include <string>
#include <vector>

enum class DataType : unsigned int { integer, real, text, Count };

std::string DataType2String(const DataType&);
DataType String2DataType(const std::string&);

enum class FieldConstraint : unsigned int {
    primary_key,
    foreign_key,
    not_null,
    unique,
    Count
};

std::string FieldConstraint2String(const FieldConstraint&);
FieldConstraint String2FieldConstraint(const std::string&);

class Field {
   public:
    Field(std::string name, const DataType type,
          const std::set<FieldConstraint>& constraints = {})
        : name_(std::move(name)),
          type_(type),
          constraint_(constraints) {}

    Field(std::string name, const DataType type,
          const std::string& constraints)
        : name_(std::move(name)),
          type_(type),
          constraint_(checkConstraints(constraints)) {}

    [[nodiscard]] DataType getType() const { return type_; };
    [[nodiscard]] std::string getName() const { return name_; };
    [[nodiscard]] std::set<FieldConstraint> getConstraint() const {
        return constraint_;
    };
    [[nodiscard]] std::vector<std::string> getData() const { return data_; };

    void addData(const std::string&);

   private:
    bool checkDataForType(const DataType type, const std::string& data);
    std::set<FieldConstraint> checkConstraints(const std::string& constraints);

    std::string name_;
    DataType type_;
    std::set<FieldConstraint> constraint_;
    std::vector<std::string> data_;
};

#endif