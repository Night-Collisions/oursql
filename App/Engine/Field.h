#ifndef OURSQL_APP_LOGIC_FIELD_H_
#define OURSQL_APP_LOGIC_FIELD_H_

#include <map>
#include <set>
#include <string>
#include <vector>

enum class DataType : unsigned int { integer, real, text, Count };

enum class FieldConstraint : unsigned int {
    primary_key,
    foreign_key,
    not_null,
    unique
};

class Field {
   public:
    Field(std::string name, const DataType type,
          const std::string& constraints)
        : name_(std::move(name)),
          type_(type),
          constraint_(checkConstraints(constraints)) {
        if (constraint_map_.empty()) {
            initMap();
        }
    }

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
    static void initMap();

    static std::map<std::string, FieldConstraint> constraint_map_;

    std::string name_;
    DataType type_;
    std::set<FieldConstraint> constraint_;
    std::vector<std::string> data_;
};

#endif