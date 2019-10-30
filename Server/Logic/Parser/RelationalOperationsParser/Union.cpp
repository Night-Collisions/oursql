#include "Union.h"
#include <map>

std::string Union::recordToConcat(const std::vector<Value>& r) {
    std::string res;
    for (auto& c : r) {
        res += c.data;
    }

    return res;
}

Table Union::makeUnion(const Table& table1, const Table& table2,
                       std::unique_ptr<exc::Exception>& e) {
    if (e) {
        return Table();
    }

    auto columns1 = table1.getColumns();
    auto columns2 = table2.getColumns();
    size_t siz1 = columns1.size();
    size_t siz2 = columns2.size();
    std::vector<std::string> col_names;
    if (siz1 == siz2) {
        for (int i = 0; i < siz1; ++i) {
            col_names.push_back(columns1[i].getName());
            if (columns1[i].getType() != columns2[i].getType() ||
                (columns1[i].getN() != columns2[i].getN())) {
                e.reset(new exc::UnionException(
                    "Columns must have the same type order.",
                    exc::ExceptionType::column_datatype_mismatch_union));
                return Table();
            }
            if (columns1[i].getConstraints().find(ColumnConstraint::not_null) ==
                columns1[i].getConstraints().end() ||
                columns2[i].getConstraints().find(ColumnConstraint::not_null) ==
                    columns2[i].getConstraints().end()) {
                e.reset(new exc::UnionException(
                    "Union requires all columns to be not null.",
                    exc::ExceptionType::null_column_in_union));
                return Table();
            }
        }
    } else {
        e.reset(
            new exc::UnionException("Tables must have the same column sizes.",
                                    exc::ExceptionType::column_sizes_union));
        return Table();
    }

    auto records1 = table1.getRecords();
    auto records2 = table2.getRecords();

    std::map<std::string, std::vector<Value>> repeated_records;
    std::vector<std::vector<Value>> unioned;

    for (auto& r : records1) {
        auto key = recordToConcat(r);
        repeated_records[key] = r;
    }

    Table table;
    table.renameColumns(col_names, e);
    if (e) {
        return Table();
    }
    for (auto& r : records2) {
        auto key = recordToConcat(r);
        if (repeated_records.find(key) == repeated_records.end()) {
            unioned.push_back(r);
        }
    }

    table.setName(table1.getName());
    for (auto& r : unioned) {
        table.addRecord(r, e);
        if (e) {
            return Table();
        }
    }

    std::vector<DataType> types;
    std::vector<int> varchar_lengths;
    std::vector<std::set<ColumnConstraint>> constraints;
    for (int i = 0; i < table1.getColumns().size(); ++i) {
        table.getColumns()[i].setType(table1.getColumns()[i].getType());
        table.getColumns()[i].setN(table1.getColumns()[i].getN());
        table.getColumns()[i].setConstraints(
            table1.getColumns()[i].getConstraints());
    }

    return table;
}
