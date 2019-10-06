#ifndef OURSQL_APP_LOGIC_FIELD_H_
#define OURSQL_APP_LOGIC_FIELD_H_

#include <map>
#include <set>
#include <string>
#include <vector>

enum class DataType : unsigned int { integer, real, text, Count };

std::string DataType2String(const DataType&);
DataType String2DataType(const std::string&);

enum class ColumnConstraint : unsigned int {
    primary_key,
    not_null,
    unique,
    Count
};

std::string ColumnConstraint2String(const ColumnConstraint&);
ColumnConstraint String2ColumnConstraint(const std::string&);

class Column {
   public:
    Column(std::string name, const DataType type,
          const std::set<ColumnConstraint>& constraints = {})
        : name_(std::move(name)), type_(type) {
        checkConstraint(constraints);
        constraint_ = constraints;
    }

    [[nodiscard]] DataType getType() const { return type_; };
    [[nodiscard]] std::string getName() const { return name_; };
    [[nodiscard]] std::set<ColumnConstraint> getConstraint() const {
        return constraint_;
    };
    [[nodiscard]] std::vector<std::string> getData() const { return data_; };

    void addData(const std::string&);

    static std::set<ColumnConstraint> checkConstraints(
        const std::string& constraints);

   private:
    static void checkConstraint(const std::set<ColumnConstraint>&);
    bool checkDataForType(const DataType type, const std::string& data);

    std::string name_;
    DataType type_;
    std::set<ColumnConstraint> constraint_;
    std::vector<std::string> data_;
};

#endif