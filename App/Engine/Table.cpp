#include "Table.h"

#include <algorithm>

void Table::addField(const Column& field) {
    auto lowerCase = [](const std::string& s) {
        auto data = s;
        std::transform(data.begin(), data.end(), data.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        return data;
    };
    for (const auto& i : fields_) {
        if (lowerCase(i.getName()) == lowerCase(field.getName())) {
            throw std::invalid_argument("Column '" + i.getName() +
                                        "' already exists");
        }
    }
    auto constraint = field.getConstraint();
    if (constraint.find(FieldConstraint::primary_key) != constraint.end()) {
        for (const auto& i : fields_) {
            auto buff = i.getConstraint();
            if (buff.find(FieldConstraint::primary_key) != buff.end()) {
                throw std::invalid_argument("Primary key already exists");
            }
        }
    }
    fields_.emplace_back(field);
}
