#ifndef OURSQL_APP_LOGIC_TABLE_H_
#define OURSQL_APP_LOGIC_TABLE_H_

#include <memory>
#include <string>
#include <vector>

#include "Column.h"

class Table {
   public:
    Table() = default;
    Table(const std::string& name, const std::vector<Column>& columns,
          std::unique_ptr<exc::Exception>& e)
        : name_(name) {
        for (auto& i : columns) {
            addColumn(i, e);
        }
    }

    void setType(DataType type, int ind) {
        columns_[ind].setType(type);
    }

    void setConstraints(std::set<ColumnConstraint> constraints, int ind) {
        columns_[ind].setConstraints(std::move(constraints));
    }
    
    void setN(int n, int ind) {
        columns_[ind].setN(n);
    }

    int getColSize() { return columns_.size(); }

    void setName(const std::string& name) { name_ = name; }

    [[nodiscard]] std::string getName() const { return name_; }
    [[nodiscard]] std::vector<Column> getColumns() const { return columns_; }

    void addColumn(const Column& column, std::unique_ptr<exc::Exception>& e);

    void renameColumns(const std::vector<std::string>& names,
                       std::unique_ptr<exc::Exception>& e) {
        if (columns_.empty()) {
            columns_.resize(names.size());
        }
        if (names.size() != columns_.size()) {
            e.reset(new exc::TableException(
                name_, exc::ExceptionType::different_column_sizes,
                "Different column size"));
            return;
        }

        for (unsigned int i = 0; i < columns_.size(); ++i) {
            columns_[i].setName(names[i]);
        }
    }

    void addRecord(const std::vector<Value>& record,
                   std::unique_ptr<exc::Exception>& e) {
        if (record.size() != columns_.size()) {
            e.reset(new exc::TableException(
                name_, exc::ExceptionType::different_column_sizes,
                "Can't add record to table. Different sizes."));
            return;
        }

        for (unsigned int i = 0; i < columns_.size(); ++i) {
            columns_[i].addValue(record[i]);
        }
    }

    std::vector<std::vector<Value>> getRecords() const {
        std::vector<std::vector<Value>> res;
        std::vector<Value> rec;
        for (unsigned int i = 0; i < columns_[0].getValues().size(); ++i) {
            rec.clear();
            for (auto& column : columns_) {
                rec.push_back(column.getValues()[i]);
            }
            res.push_back(rec);
        }

        return res;
    }

   private:
    std::string name_;
    std::vector<Column> columns_;
};

#endif
