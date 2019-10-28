#include "Join.h"
#include <iostream>
#include <map>
#include "../ExpressionParser/Resolver.h"

Table Join::makeJoin(const Table& table1, const Table& table2,
                     Expression* on_expr, std::unique_ptr<exc::Exception>& e,
                     RelOperNodeType join_type) {
    if (e) {
        return Table();
    }

    if (join_type == RelOperNodeType::right_join) {
        auto res =
            makeJoin(table2, table1, on_expr, e, RelOperNodeType::left_join);
        if (e) {
            return Table();
        }
        return res;
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
    for (auto& c : table1.getColumns()) {
        col_names.push_back(table1.getName() + "." + c.getName());
    }
    for (auto& c : table2.getColumns()) {
        col_names.push_back(table2.getName() + "." + c.getName());
    }
    table.renameColumns(col_names, e);
    if (e) {
        return Table();
    }

    auto use_hash = isHashJoinOk(table1.getName(), table2.getName(),
                                 column_info, on_expr, e);
    if (e) {
        return Table();
    }

    /*    checkColumns(table1, table2, on_expr, e);
        if (e) {
            return Table();
        }*/

    if ((siz1 * siz2 > siz1 + siz2) && use_hash) {
        std::string col1 = on_expr->childs()[0]->getConstant()->getName();
        std::string col2 = on_expr->childs()[1]->getConstant()->getName();
        if (column_info[table1.getName()].find(col1) ==
            column_info[table1.getName()].end()) {
            std::swap(col1, col2);
        }
        std::vector<std::vector<Value>> record_to_hash;
        std::vector<std::vector<Value>> record_to_run;
        std::string key_col;
        int pos_right = 0;
        int pos_left = 0;
        std::string tablename_to_run;
        std::string tablename_to_hash;
        Table table_to_run;
        Table table_to_hash;
        if (siz1 > siz2) {
            record_to_hash = records2;
            record_to_run = records1;
            key_col = col2;
            pos_right = std::distance(column_info[table2.getName()].find(col1),
                                      column_info[table2.getName()].begin());
            pos_left = std::distance(column_info[table1.getName()].find(col2),
                                     column_info[table1.getName()].begin());
            tablename_to_run = table1.getName();
            tablename_to_hash = table2.getName();
            table_to_run = table1;
            table_to_hash = table2;
        } else {
            record_to_hash = records1;
            record_to_run = records2;
            key_col = col1;
            pos_right = std::distance(column_info[table1.getName()].find(col2),
                                      column_info[table1.getName()].begin());
            pos_left = std::distance(column_info[table2.getName()].find(col1),
                                     column_info[table2.getName()].begin());
            tablename_to_run = table2.getName();
            tablename_to_hash = table1.getName();
            table_to_run = table2;
            table_to_hash = table1;
        }

        std::map<std::string, std::vector<std::vector<Value>>> hashed_records;
        DataType type_right = table_to_hash.getColumns()[pos_right].getType();
        DataType type_left = table_to_run.getColumns()[pos_left].getType();
        for (auto& r : record_to_hash) {
            std::string key;
            if (type_right == DataType::integer) {
                std::cout << r[pos_right].data << std::endl;
                key = std::to_string(std::stof(r[pos_right].data));
            } else {
                key = r[pos_right].data;
            }
            hashed_records[key].push_back(r);
        }

        for (auto& r : record_to_run) {
            std::string key;
            if (type_left == DataType::integer) {
                key = std::to_string(std::stof(r[pos_right].data));
            } else {
                key = r[pos_right].data;
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
                    std::vector<Value> new_rec;
                    std::copy(rec1.begin(), rec1.end(),
                              std::back_inserter(new_rec));
                    std::copy(rec2.begin(), rec2.end(),
                              std::back_inserter(new_rec));
                    table.addRecord(new_rec, e);
                    if (e) {
                        return Table();
                    }
                } else {
                    continue;
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
    } else {
        if (child1->getNodeType() == NodeType::ident && !child1->childs()[0] &&
            !child1->childs()[1]) {
            return false;
        }
    }
    if (!child2) {
        return false;
    } else {
        if (child2->getNodeType() == NodeType::ident && !child2->childs()[0] &&
            !child2->childs()[1]) {
            return false;
        }
    }

    if (!Resolver::compareTypes(name1, name2, column_info,
                                child1->getConstant(), child2->getConstant(), e,
                                CompareCondition::compare, "=")) {
        return false;
    }

    /*    if (name1 == name2 &&
            child1->getConstant()->getNodeType() == NodeType::ident &&
            child2->getConstant()->getNodeType() == NodeType::ident &&
            static_cast<Ident*>(child1->getConstant())->getTableName() ==
                static_cast<Ident*>(child2->getConstant())->getTableName() &&
            static_cast<Ident*>(child1->getConstant())->getName() ==
                static_cast<Ident*>(child2->getConstant())->getName()) {
            e.reset(new exc::AmbiguousColumnName(
                "ambiguous column name " +
                static_cast<Ident*>(child1->getConstant())->getName()));
            return false;
        }*/

    if (column_info[name1].find(child1->getConstant()->getName()) !=
            column_info[name1].end() ||
        column_info[name2].find(child1->getConstant()->getName()) !=
            column_info[name2].end()) {
        ok1 = true;
    }

    if (column_info[name1].find(child2->getConstant()->getName()) !=
            column_info[name1].end() ||
        column_info[name2].find(child2->getConstant()->getName()) !=
            column_info[name2].end()) {
        ok2 = true;
    }

    if (expr->exprType() == ExprUnit::equal) {
        return ok1 && ok2;
    }

    return false;
}

void Join::checkColumns(const Table& table1, const Table& table2,
                        Expression* on_expr,
                        std::unique_ptr<exc::Exception>& e) {
    if (table1.getName() == table2.getName()) {
    }
}
