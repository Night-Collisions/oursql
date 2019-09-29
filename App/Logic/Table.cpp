#include "Table.h"

void Table::addField(const Field &field) {
    fields_.emplace_back(field);
}
