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
std::string ColumnConstraint2String(unsigned char constraint);
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

    Column(std::string name, const DataType type,
           std::unique_ptr<exc::Exception>& e, unsigned char constraint)
        : name_(std::move(name)), type_(type) {
        auto constraints = bitConstraintToSet(constraint);
        constraint_ = constraints;
    }

    [[nodiscard]] DataType getType() const { return type_; };
    [[nodiscard]] std::string getName() const { return name_; };
    [[depricated]] const std::set<ColumnConstraint>& getConstraints() const {
        return constraint_;
    };

    unsigned char getBitConstraint() const {
        unsigned char res = 0;
        for (auto& c : constraint_) {
            res |= 1 << static_cast<unsigned char>(c);
        }

        return res;
    }

    static std::set<ColumnConstraint> bitConstraintToSet(unsigned char constraint) {
        std::set<ColumnConstraint> res;
        ColumnConstraint arr[]{ColumnConstraint::primary_key,
                               ColumnConstraint::not_null,
                               ColumnConstraint::unique};
        for (int i = 0; i < 8; ++i) {
            if ((constraint & (1 << i)) != 0) {
                res.insert(arr[i]);
            }
        }

        return res;
    }

    [[nodiscard]] std::vector<std::string> getData() const { return data_; };

    void addData(const std::string&, std::unique_ptr<exc::Exception>&);

    void setN(int n) { n_ = n; }
    int getN() const { return n_; }

   private:
    static bool checkConstraint(const std::set<ColumnConstraint>&,
                                std::pair<ColumnConstraint, ColumnConstraint>&);
    bool checkDataForType(const DataType type, const std::string& data);

    std::string name_;
    DataType type_;
    std::set<ColumnConstraint> constraint_;
    std::vector<std::string> data_;
    int n_;
};

#endif