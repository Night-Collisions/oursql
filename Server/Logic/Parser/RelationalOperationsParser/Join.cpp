#include "Join.h"
#include <iostream>
#include <map>
#include "../ExpressionParser/Resolver.h"
#include "Helper.h"

Table Join::makeJoin(const Table& table1, const Table& table2,
                     Expression* on_expr, std::unique_ptr<exc::Exception>& e,
                     RelOperNodeType type) {
    if (e) {
        return Table();
    }

    if (table1.getName() == table2.getName()) {
        e.reset(new exc::AmbiguousColumnName("ambiguous column name " +
                                             table1.getColumns()[0].getName()));
        return Table();
    }

    std::map<std::string, std::map<std::string, Column>> column_info;
    for (auto& c : table1.getColumns()) {
        column_info[table1.getName()][c.getName()] = c;
    }
    for (auto& c : table2.getColumns()) {
        column_info[table2.getName()][c.getName()] = c;
    }

    std::map<std::string, std::map<std::string, std::string>> record_info;

    auto records1 = table1.getRecords();
    auto records2 = table2.getRecords();
    auto siz1 = records1.size();
    auto siz2 = records2.size();
    Table table;
    std::vector<std::string> col_names;

    if (e) {
        return Table();
    }

    auto use_hash = isHashJoinOk(table1.getName(), table2.getName(),
                                 column_info, on_expr, e);
    if (e) {
        return Table();
    }

    if ((siz1 * siz2 > siz1 + siz2) && use_hash &&
        (type == RelOperNodeType::join ||
         type == RelOperNodeType::inner_join)) {
        std::string col_of_run = on_expr->childs()[0]->getConstant()->getName();
        std::string col_of_hash =
            on_expr->childs()[1]->getConstant()->getName();
        if (table1.getName() !=
            static_cast<Ident*>(on_expr->childs()[0]->getConstant())
                ->getTableName()) {
            std::swap(col_of_run, col_of_hash);
        }

        std::vector<std::vector<Value>> records_to_hash;
        std::vector<std::vector<Value>> records_to_run;
        std::string key_col;
        int pos_of_hash = 0;
        int pos_of_run = 0;
        std::string tablename_to_run;
        std::string tablename_to_hash;
        Table table_to_run;
        Table table_to_hash;
        key_col = col_of_hash;
        records_to_run = records1;
        records_to_hash = records2;
        table_to_run = table1;
        table_to_hash = table2;
        tablename_to_run = table_to_run.getName();
        tablename_to_hash = table_to_hash.getName();
        setPosOfTheCols(table_to_run, table_to_hash, col_of_run, col_of_hash,
                        pos_of_run, pos_of_hash);
        if (siz1 < siz2) {
            key_col = col_of_run;
            std::swap(table_to_run, table_to_hash);
            std::swap(tablename_to_run, tablename_to_hash);
            std::swap(pos_of_run, pos_of_hash);
            std::swap(records_to_run, records_to_hash);
        }

        std::map<std::string, std::vector<std::vector<Value>>> hashed_records;
        DataType type_right = table_to_hash.getColumns()[pos_of_hash].getType();
        DataType type_left = table_to_run.getColumns()[pos_of_run].getType();
        for (auto& r : records_to_hash) {
            std::string key;
            if (type_right == DataType::integer) {
                std::cout << r[pos_of_hash].data << std::endl;
                key = std::to_string(std::stof(r[pos_of_hash].data));
            } else {
                key = r[pos_of_hash].data;
            }
            hashed_records[key].push_back(r);
        }
        std::vector<DataType> types;
        std::vector<int> varchar_sizes;
        std::vector<std::set<ColumnConstraint>> constraints;
        pushBackMeta(table_to_run.getName(), table_to_run.getColumns(),
                     col_names, types, varchar_sizes, constraints);
        pushBackMeta(table_to_hash.getName(), table_to_hash.getColumns(),
                     col_names, types, varchar_sizes, constraints);

        for (auto s : constraints) {
            s.insert(ColumnConstraint::not_null);
        }

        initTable(table, col_names, types, varchar_sizes, constraints, e);
        if (e) {
            return Table();
        }

        for (auto& r : records_to_run) {
            std::string key;
            if (type_left == DataType::integer) {
                key = std::to_string(std::stof(r[pos_of_run].data));
            } else {
                key = r[pos_of_run].data;
            }
            if (hashed_records.find(key) != hashed_records.end()) {
                for (auto& v : hashed_records[key]) {
                    std::vector<Value> new_rec;
                    std::copy(r.begin(), r.end(), std::back_inserter(new_rec));
                    std::copy(v.begin(), v.end(), std::back_inserter(new_rec));
                    table.addRecord(new_rec, e);
                    if (e) {
                        return Table();
                    }
                }
            }
        }

    } else {
        std::vector<std::vector<Value>> record_arr;
        std::vector<DataType> types;
        std::vector<Value> null_vector_left =
            getNullRecord(table1.getColumns());
        std::vector<Value> null_vector_right =
            getNullRecord(table2.getColumns());
        std::vector<int> varchar_sizes;
        std::vector<std::set<ColumnConstraint>> constraints;
        pushBackMeta(table1.getName(), table1.getColumns(), col_names, types,
                     varchar_sizes, constraints);
        pushBackMeta(table2.getName(), table2.getColumns(), col_names, types,
                     varchar_sizes, constraints);
        if (type == RelOperNodeType::join ||
            type == RelOperNodeType::inner_join) {
            for (auto s : constraints) {
                s.insert(ColumnConstraint::not_null);
            }
        }
        initTable(table, col_names, types, varchar_sizes, constraints, e);
        if (e) {
            return Table();
        }
        std::vector<std::set<int>> used;
        used.resize(2);

        for (int i = 0; i < siz1; ++i) {
            auto rec1 = records1[i];
            for (int j = 0; j < siz2; ++j) {
                auto rec2 = records2[j];
                record_info[table1.getName()] =
                    Resolver::getRecordMap(table1.getColumns(), rec1, e);
                if (e) {
                    return Table();
                }
                record_info[table2.getName()] =
                    Resolver::getRecordMap(table2.getColumns(), rec2, e);
                if (e) {
                    return Table();
                }

                auto res =
                    Resolver::resolve(table1.getName(), table2.getName(),
                                      column_info, on_expr, record_info, e);
                if (e) {
                    return Table();
                }

                if (res != "0") {
                    used[0].insert(i);
                    used[1].insert(j);
                    std::vector<Value> new_rec;
                    std::copy(rec1.begin(), rec1.end(),
                              std::back_inserter(new_rec));
                    std::copy(rec2.begin(), rec2.end(),
                              std::back_inserter(new_rec));
                    table.addRecord(new_rec, e);
                    if (e) {
                        return Table();
                    }
                }
            }
        }
        if (type == RelOperNodeType::left_join ||
            type == RelOperNodeType::full_join) {
            for (int i = 0; i < records1.size(); ++i) {
                if (used[0].find(i) == used[0].end()) {
                    std::vector<Value> new_rec;
                    std::copy(records1[i].begin(), records1[i].end(),
                              std::back_inserter(new_rec));
                    std::copy(null_vector_right.begin(),
                              null_vector_right.end(),
                              std::back_inserter(new_rec));
                    table.addRecord(new_rec, e);
                    if (e) {
                        return Table();
                    }
                }
            }
        }
        if (type == RelOperNodeType::right_join ||
            type == RelOperNodeType::full_join) {
            for (int i = 0; i < records2.size(); ++i) {
                if (used[1].find(i) == used[1].end()) {
                    std::vector<Value> new_rec;
                    std::copy(null_vector_left.begin(), null_vector_left.end(),
                              std::back_inserter(new_rec));
                    std::copy(records2[i].begin(), records2[i].end(),
                              std::back_inserter(new_rec));
                    table.addRecord(new_rec, e);
                    if (e) {
                        return Table();
                    }
                }
            }
        }
    }

    return table;
}

bool Join::isHashJoinOk(
    const std::string& name1, const std::string& name2,
    std::map<std::string, std::map<std::string, Column>> column_info,
    Expression* expr, std::unique_ptr<exc::Exception>& e) {
    bool ok1 = false;
    bool ok2 = false;

    if (!expr) {
        return false;
    }
    if (expr->exprType() != ExprUnit::equal) {
        return false;
    }

    auto child1 = expr->childs()[0];
    auto child2 = expr->childs()[1];

    if (!child1) {
        return false;
    }

    if (!child2) {
        return false;
    }

    if (static_cast<Ident*>(child1->getConstant())->getNodeType() != NodeType::ident) {
        return false;
    }

    if (static_cast<Ident*>(child2->getConstant())->getNodeType() != NodeType::ident) {
        return false;
    }

    if (static_cast<Ident*>(child1->getConstant())->getNodeType() == NodeType::ident && !child2->childs()[0] &&
        !child2->childs()[1]) {
        ok2 = true;
    }

    if (static_cast<Ident*>(child2->getConstant())->getNodeType() == NodeType::ident && !child2->childs()[0] &&
        !child2->childs()[1]) {
        ok2 = true;
    }

    // TODO: когда имя таблицы пустое, рассматривать соединенное имя

    auto tbl_name1 = name1;
    auto tbl_name2 = name2;

    if (name1.empty()) {
        // tbl_name1 =
        // static_cast<Ident*>(child1->getConstant())->getTableName();
        auto t = static_cast<Ident*>(child1->getConstant())->getTableName();
        auto c = static_cast<Ident*>(child1->getConstant())->getName();
        static_cast<Ident*>(child1->getConstant())->setTableName("");
        static_cast<Ident*>(child1->getConstant())
            ->setName(Helper::getCorrectTablePrefix(t) + c);
    }
    if (name2.empty()) {
        auto t = static_cast<Ident*>(child2->getConstant())->getTableName();
        auto c = static_cast<Ident*>(child2->getConstant())->getName();
        static_cast<Ident*>(child2->getConstant())->setTableName("");
        static_cast<Ident*>(child2->getConstant())
            ->setName(Helper::getCorrectTablePrefix(t) + c);
    }

    if (!Resolver::compareTypes(tbl_name1, tbl_name2, column_info,
                                child1->getConstant(), child2->getConstant(), e,
                                CompareCondition::compare, "=")) {
        return false;
    }

    if (column_info[tbl_name1].find(child1->getConstant()->getName()) !=
            column_info[tbl_name1].end() ||
        column_info[tbl_name2].find(child1->getConstant()->getName()) !=
            column_info[tbl_name2].end()) {
        ok1 = true;
    }

    if (column_info[tbl_name1].find(child2->getConstant()->getName()) !=
            column_info[tbl_name1].end() ||
        column_info[tbl_name2].find(child2->getConstant()->getName()) !=
            column_info[tbl_name2].end()) {
        ok2 = true;
    }

    if (expr->exprType() == ExprUnit::equal) {
        return ok1 && ok2;
    }

    return false;
}

std::vector<Value> Join::getNullRecord(const std::vector<Column>& cols) {
    std::vector<Value> res;

    for (auto& c : cols) {
        Value v;
        if (c.getType() == DataType::varchar) {
            v.data = "";
            v.is_null = true;
        } else {
            v.data = "null";
            v.is_null = true;
        }
        res.push_back(v);
    }

    return res;
}

void Join::pushBackMeta(const std::string& table_name,
                        const std::vector<Column>& cols,
                        std::vector<std::string>& col_names,
                        std::vector<DataType>& types,
                        std::vector<int>& varchar_sizes,
                        std::vector<std::set<ColumnConstraint>>& constraints) {
    for (auto& c : cols) {
        col_names.push_back(Helper::getCorrectTablePrefix(table_name) +
                            c.getName());
        types.push_back(c.getType());
        varchar_sizes.push_back(c.getN());
        constraints.push_back(c.getConstraints());
    }
}

void Join::initTable(Table& table, const std::vector<std::string>& col_names,
                     const std::vector<DataType>& types,
                     const std::vector<int>& varchar_sizes,
                     const std::vector<std::set<ColumnConstraint>>& constraints,
                     std::unique_ptr<exc::Exception>& e) {
    table.renameColumns(col_names, e);
    if (e) {
        return;
    }
    for (int i = 0; i < table.getColSize(); ++i) {
        table.setType(types[i], i);
        table.setN(varchar_sizes[i], i);
        table.setConstraints(constraints[i], i);
    }
}

void Join::setPosOfTheCols(Table& to_run, Table& to_hash,
                           const std::string& col_of_run,
                           const std::string& col_of_hash, int& pos_of_run,
                           int& pos_of_hash) {
    pos_of_hash = pos_of_run = 0;
    for (auto& c : to_run.getColumns()) {
        if (c.getName() == col_of_run) {
            break;
        }
        ++pos_of_run;
    }
    for (auto& c : to_hash.getColumns()) {
        if (c.getName() == col_of_hash) {
            break;
        }
        ++pos_of_hash;
    }
}
