#include "Table.h"

#include <algorithm>

void Table::addColumn(const Column& column, exc::Exception* e) {
    RESET_EXCEPTION(e);
    auto lowerCase = [](const std::string& s) {
        auto data = s;
        std::transform(data.begin(), data.end(), data.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        return data;
    };
    for (const auto& i : columns_) {
        if (lowerCase(i.getName()) == lowerCase(column.getName())) {
            SET_EXCEPTION(e, exc::RepeatColumnName(name_, i.getName()));
            return;
        }
    }
    auto constraint = column.getConstraint();
    if (constraint.find(ColumnConstraint::primary_key) != constraint.end()) {
        for (const auto& i : columns_) {
            auto buff = i.getConstraint();
            if (buff.find(ColumnConstraint::primary_key) != buff.end()) {
                SET_EXCEPTION(e, exc::constr::DuplicatedPrimaryKey(
                                     name_, i.getName(), column.getName()));
                throw std::invalid_argument("Primary key already exists");
            }
        }
    }
    columns_.emplace_back(column);
}
