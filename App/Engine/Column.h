#ifndef OURSQL_APP_LOGIC_FIELD_H_
#define OURSQL_APP_LOGIC_FIELD_H_

#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "../Core/DataType.h"
#include "../Core/Exception.h"

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
    Column() = default;
    Column(std::string name, const DataType type,
           std::unique_ptr<exc::Exception>& e,
           const std::set<ColumnConstraint>& constraints = {})
        : name_(std::move(name)), type_(type) {
        std::pair<ColumnConstraint, ColumnConstraint> err_constraints;
        e.reset(nullptr);
        if (checkConstraint(constraints, err_constraints)) {
            e.reset(new exc::constr::IncompatibleConstraints(
                name_, ColumnConstraint2String(err_constraints.first),
                ColumnConstraint2String(err_constraints.second)));
        }
        constraint_ = constraints;
    }

    [[nodiscard]] DataType getType() const { return type_; };
    [[nodiscard]] std::string getName() const { return name_; };
    const std::set<ColumnConstraint>& getConstraint() const {
        return constraint_;
    };
    [[nodiscard]] std::vector<std::string> getData() const { return data_; };

    void addData(const std::string&, std::unique_ptr<exc::Exception>&);
    bool isNull() { return is_null_; }

   private:
    static bool checkConstraint(const std::set<ColumnConstraint>&,
                                std::pair<ColumnConstraint, ColumnConstraint>&);
    bool checkDataForType(const DataType type, const std::string& data);

    std::string name_;
    DataType type_;
    std::set<ColumnConstraint> constraint_;
    std::vector<std::string> data_;
    bool is_null_{false};
};

#endif