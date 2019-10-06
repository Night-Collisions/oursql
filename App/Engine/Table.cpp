#include "Table.h"

#include <algorithm>

void Table::addColumn(const Column& column) {
    auto lowerCase = [](const std::string& s) {
        auto data = s;
        std::transform(data.begin(), data.end(), data.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        return data;
    };
    for (const auto& i : columns_) {
        if (lowerCase(i.getName()) == lowerCase(column.getName())) {
            throw std::invalid_argument("Column '" + i.getName() +
                                        "' already exists");
        }
    }
    auto constraint = column.getConstraint();
    if (constraint.find(ColumnConstraint::primary_key) != constraint.end()) {
        for (const auto& i : columns_) {
            auto buff = i.getConstraint();
            if (buff.find(ColumnConstraint::primary_key) != buff.end()) {
                throw std::invalid_argument("Primary key already exists");
            }
        }
    }
    columns_.emplace_back(column);
}
