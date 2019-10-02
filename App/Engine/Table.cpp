#include "Table.h"

void Table::addField(const Field& field) {
    for (const auto& i : fields_) {
        if (i.getName() == field.getName()) {
            throw std::invalid_argument("Field '" + i.getName() +
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
