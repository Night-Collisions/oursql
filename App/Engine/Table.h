#ifndef OURSQL_APP_LOGIC_TABLE_H_
#define OURSQL_APP_LOGIC_TABLE_H_

#include <string>
#include <vector>

#include "Column.h"

class Table {
   public:
    Table() = default;
    Table(const std::string& name, const std::vector<Column>& columns)
        : name_(name), columns_(columns) {}

    [[deprecated]] void setName(const std::string& name) { name_ = name; }

    [[nodiscard]] std::string getName() const { return name_; }
    [[nodiscard]] std::vector<Column> getColumns() const { return columns_; }

    [[deprecated]] void addColumn(const Column& column, exc::Exception* e);

   private:
    std::string name_;
    std::vector<Column> columns_;
};

#endif
