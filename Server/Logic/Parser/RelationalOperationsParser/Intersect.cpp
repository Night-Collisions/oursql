#include "Intersect.h"
#include <map>
#include "Helper.h"
#include "Union.h"

Table Intersect::makeIntersect(const Table& table1, const Table& table2,
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
            if (!Helper::checkTypes(columns1[i].getType(),
                                    columns2[i].getType()) ||
                (columns1[i].getN() != columns2[i].getN())) {
                e.reset(new exc::IntersectException(
                    "Columns must have the same type order.",
                    exc::ExceptionType::column_datatype_mismatch_intersect));
                return Table();
            }
            if (columns1[i].getConstraints().find(ColumnConstraint::not_null) ==
                    columns1[i].getConstraints().end() ||
                columns2[i].getConstraints().find(ColumnConstraint::not_null) ==
                    columns2[i].getConstraints().end()) {
                e.reset(new exc::IntersectException(
                    "Intersect requires all columns to be not null.",
                    exc::ExceptionType::null_column_in_intersect));
                return Table();
            }
        }
    } else {
        e.reset(new exc::IntersectException(
            "Tables must have the same column sizes.",
            exc::ExceptionType::column_sizes_intersect));
        return Table();
    }

    auto records1 = table1.getRecords();
    auto records2 = table2.getRecords();

    std::map<std::string, std::vector<Value>> repeated_records;
    std::vector<std::vector<Value>> intersected;

    for (auto& r : records1) {
        auto key = Helper::recordToConcat(r);
        repeated_records[key] = r;
    }

    Table table;
    table.renameColumns(col_names, e);
    if (e) {
        return Table();
    }
    std::map<std::string, std::vector<Value>> repeated2;
    for (auto& r : records2) {
        auto key = Helper::recordToConcat(r);
        if (repeated_records.find(key) != repeated_records.end() && repeated2.find(key) == repeated2.end()) {
            intersected.push_back(r);
            repeated2[key] = r;
        }
    }

    table.setName(table1.getName());
    for (auto& r : intersected) {
        table.addRecord(r, e);
        if (e) {
            return Table();
        }
    }

    std::vector<DataType> types;
    std::vector<int> varchar_lengths;
    std::vector<std::set<ColumnConstraint>> constraints;
    for (int i = 0; i < table1.getColumns().size(); ++i) {
        table.setType(table1.getColumns()[i].getType(), i);
        table.setN(table1.getColumns()[i].getN(), i);
        table.setConstraints(table1.getColumns()[i].getConstraints(), i);
    }

    return table;
}
