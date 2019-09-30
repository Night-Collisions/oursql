#ifndef OURSQL_APP_LOGIC_TABLE_H_
#define OURSQL_APP_LOGIC_TABLE_H_

#include <string>
#include <vector>

#include "Field.h"

class Table {
   public:
    void setName(const std::string& name) { name_ = name; }

    std::string getName() const { return name_; }
    std::vector<Field> getFields() const { return fields_; }

    void addField(const Field& field);

   private:
    std::string name_;
    std::vector<Field> fields_;
};

#endif
