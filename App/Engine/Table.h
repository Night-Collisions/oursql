#ifndef OURSQL_APP_LOGIC_TABLE_H_
#define OURSQL_APP_LOGIC_TABLE_H_

#include <string>
#include <vector>
#include <memory>

#include "Column.h"

class Table {
   public:
    Table() = default;
    Table(const std::string& name, const std::vector<Column>& columns, std::unique_ptr<exc::Exception>& e)
        : name_(name) { for (auto& i : columns) { addColumn(i, e); }}

    [[deprecated]] void setName(const std::string& name) { name_ = name; }

    [[nodiscard]] std::string getName() const { return name_; }
    [[nodiscard]] std::vector<Column> getColumns() const { return columns_; }

    void addColumn(const Column& column, std::unique_ptr<exc::Exception>& e);

   private:
    std::string name_;
    std::vector<Column> columns_;
};

#endif
