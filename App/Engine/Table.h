#ifndef OURSQL_APP_LOGIC_TABLE_H_
#define OURSQL_APP_LOGIC_TABLE_H_

#include <string>
#include <vector>

#include "Field.h"

class Table {
   public:
    Table() = default;
    Table(const std::string& name, const std::vector<Field>& fields)
        : name_(name), fields_(fields) {}

    [[deprecated]] void setName(const std::string& name) { name_ = name; }

    [[nodiscard]] std::string getName() const { return name_; }
    [[nodiscard]] std::vector<Field> getFields() const { return fields_; }

    [[deprecated]] void addField(const Field& field);

   private:
    std::string name_;
    std::vector<Field> fields_;
};

#endif
