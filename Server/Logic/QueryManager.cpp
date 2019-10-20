#include "QueryManager.h"
#include <iostream>
#include <memory>

#include "../Engine/Column.h"
#include "../Engine/Cursor.h"
#include "../Engine/Engine.h"
#include "Parser/Nodes/Ident.h"
#include "Parser/Nodes/VarList.h"

#include "Conditions/ConditionChecker.h"
#include "Parser/ExpressionParser/Resolver.h"
#include "Parser/Nodes/ConstantList.h"
#include "Parser/Nodes/IdentList.h"
#include "Parser/Nodes/SelectList.h"

void QueryManager::execute(const Query& query,
                           std::unique_ptr<exc::Exception>& e,
                           std::ostream& out) {
    void (*const
              commandsActions[static_cast<unsigned int>(CommandType::Count)])(
        const Query& query, std::unique_ptr<exc::Exception>& e,
        std::ostream& out) = {
        [](const Query&, std::unique_ptr<exc::Exception>& e,
           std::ostream& out) { assert(false); },
        createTable,
        showCreateTable,
        dropTable,
        select,
        insert,
        update,
        remove};
    CommandType command = query.getCmdType();
    if (command != CommandType::Count) {
        commandsActions[static_cast<unsigned int>(command)](query, e, out);
    }
}

void QueryManager::createTable(const Query& query,
                               std::unique_ptr<exc::Exception>& e,
                               std::ostream& out) {
    e.reset(nullptr);

    std::string name = query.getChildren()[NodeType::ident]->getName();

    auto vars = static_cast<VarList*>(query.getChildren()[NodeType::var_list])
                    ->getVars();

    std::vector<Column> columns;
    for (auto& v : vars) {
        std::string col_name = v->getName();
        DataType type = v->getType();
        int len = 0;
        if (type == DataType::varchar) {
            len = v->getVarcharLen();
        }

        std::set<ColumnConstraint> constr_set;

        for (auto& c : v->getConstraints()) {
            if (constr_set.find(c) == constr_set.end()) {
                constr_set.insert(c);
            } else {
                e.reset(
                    new exc::constr::RedundantConstraints(name, v->getName()));
                return;
            }
        }

        Column col(col_name, type, e, constr_set);
        col.setN(len);
        columns.emplace_back(col);
    }

    Table table(name, columns, e);
    if (e != nullptr) {
        return;
    }

    Engine::create(table, e);
}

void QueryManager::showCreateTable(const Query& query,
                                   std::unique_ptr<exc::Exception>& e,
                                   std::ostream& out) {
    auto name = query.getChildren()[NodeType::ident]->getName();
    auto res = Engine::showCreate(name, e);
    out << res << std::endl;
}

void QueryManager::dropTable(const Query& query,
                             std::unique_ptr<exc::Exception>& e,
                             std::ostream& out) {
    auto name = query.getChildren()[NodeType::ident]->getName();
    Engine::drop(name, e);
}

void QueryManager::select(const Query& query,
                          std::unique_ptr<exc::Exception>& e,
                          std::ostream& out) {
    auto name = query.getChildren()[NodeType::ident]->getName();
    auto table = Engine::show(name, e);
    if (table.getName().empty()) {
        e.reset(new exc::acc::TableNonexistent(name));
        return;
    }

    std::map<std::string, Column> all_columns;
    for (auto& c : table.getColumns()) {
        all_columns[c.getName()] = c;
    }

    auto cols_from_parser =
        static_cast<SelectList*>(query.getChildren()[NodeType::select_list])
            ->getList();

    std::set<std::string> asterisk;
    for (auto& m : table.getColumns()) {
        asterisk.insert(m.getName());
    }

    std::vector<Expression*> col_expr;

    for (auto& c : cols_from_parser) {
        if (c->getNodeType() == NodeType::expression_unit) {
            col_expr.push_back(static_cast<Expression*>(
                query.getChildren()[NodeType::expression]));
        }
        if (c->getName() != "*" &&
            c->getNodeType() != NodeType::expression_unit &&
            all_columns.find(c->getName()) == all_columns.end()) {
            e.reset(new exc::acc::ColumnNonexistent(c->getName(), name));
            return;
        }
    }

    if (e != nullptr) {
        return;
    }

    Cursor cursor(name);
    while (cursor.next()) {
        auto ftch = cursor.fetch();
        std::map<std::string, std::string> m =
            mapFromFetch(table.getColumns(), ftch);
        auto root =
            static_cast<Expression*>(query.getChildren()[NodeType::expression]);
        std::string response = Resolver::resolve(name, all_columns, root, m, e);
        if (e) {
            return;
        }
        int expr_cnt = 1;
        if (response != "0") {
            for (auto& c : cols_from_parser) {
                if (c->getNodeType() == NodeType::expression_unit) {
                    auto expr = static_cast<Expression*>(c);
                    response = Resolver::resolve(name, all_columns, expr, m, e);
                    if (e) {
                        return;
                    }
                    out << "expression " + std::to_string(expr_cnt++) + ": " +
                               response
                        << std::endl;
                } else if (c->getName() == "*") {
                    for (auto& k : table.getColumns()) {
                        out << k.getName() + ": " + m[k.getName()] << std::endl;
                    }
                } else {
                    out << c->getName() + ": " + m[c->getName()] << std::endl;
                }
            }
        }
    }
}

void QueryManager::insert(const Query& query,
                          std::unique_ptr<exc::Exception>& e,
                          std::ostream& out) {
    e.reset(nullptr);

    auto name = query.getChildren()[NodeType::ident]->getName();
    auto table = Engine::show(name, e);
    if (table.getName().empty()) {
        e.reset(new exc::acc::TableNonexistent(name));
        return;
    }

    std::map<std::string, Column> all_columns;
    for (auto& c : table.getColumns()) {
        all_columns[c.getName()] = c;
    }

    auto idents =
        static_cast<IdentList*>(query.getChildren()[NodeType::ident_list])
            ->getIdents();
    std::set<std::string> col_set;
    if (!idents.empty()) {
        for (auto& c : idents) {
            if (col_set.find(c->getName()) == col_set.end()) {
                col_set.insert(c->getName());
            } else {
                e.reset(new exc::RepeatColumn(c->getName()));
                return;
            }
        }
    } else {
        for (auto& c : table.getColumns()) {
            col_set.insert(c.getName());
            idents.push_back(new Ident(c.getName()));
        }
    }

    auto constants =
        static_cast<ConstantList*>(query.getChildren()[NodeType::constant_list])
            ->getConstants();

    if (constants.size() > idents.size()) {
        e.reset(new exc::ins::ConstantsMoreColumns());
        return;
    }

    std::map<std::string, std::string> values;

    for (auto& c : table.getColumns()) {
        if ((col_set.find(c.getName()) == col_set.end() &&
             c.getConstraints().find(ColumnConstraint::not_null) !=
                 c.getConstraints().end()) ||
            (col_set.find(c.getName()) == col_set.end() &&
             c.getConstraints().find(ColumnConstraint::primary_key) !=
                 c.getConstraints().end())) {
            e.reset(new exc::constr::NullNotNull(name, c.getName()));
            return;
        }
    }

    for (size_t i = 0; i < constants.size(); ++i) {
        if (Resolver::compareTypes(name, all_columns, idents[i], constants[i],
                                   e, true)) {
            if (all_columns[idents[i]->getName()].getType() ==
                    DataType::varchar &&
                all_columns[idents[i]->getName()].getN() <
                    static_cast<Constant*>(constants[i])->getValue().length()) {
                e.reset(new exc::DataTypeOversize(idents[i]->getName()));
                return;
            }
            if (static_cast<Constant*>(constants[i])->getDataType() !=
                DataType::null_) {
                auto val = static_cast<Constant*>(constants[i])->getValue();
                if (all_columns[idents[i]->getName()].getType() ==
                    DataType::real) {
                    values[idents[i]->getName()] =
                        std::to_string(std::stof(val));
                } else {
                    values[idents[i]->getName()] = std::string(val.c_str());
                }
            }
        } else {
            return;
        }
    }

    Cursor cursor(name);
    std::vector<std::vector<Value>> fetch_arr;
    while (cursor.next()) {
        fetch_arr.push_back(cursor.fetch());
    }

    std::vector<Value> v_arr;
    for (auto& c : table.getColumns()) {
        if (values.find(c.getName()) == values.end()) {
            if ((c.getConstraints().find(ColumnConstraint::primary_key) !=
                 c.getConstraints().end()) ||
                (c.getConstraints().find(ColumnConstraint::not_null) !=
                 c.getConstraints().end())) {
                e.reset(new exc::constr::NullNotNull(name, c.getName()));
                return;
            }
            Value v;
            if (c.getType() == DataType::varchar) {
                v.data = "";
            } else {
                v.data = "0";
            }
            v.is_null = true;
            v_arr.push_back(v);
        } else {
            Value v;
            v.is_null = false;
            v.data = values[c.getName()];
            v_arr.push_back(v);
        }
    }

    auto tbl_cols = table.getColumns();
    for (auto& f : fetch_arr) {
        for (int i = 0; i < f.size(); ++i) {
            if (f[i].data == v_arr[i].data &&
                ((all_columns[tbl_cols[i].getName()].getConstraints().find(
                      ColumnConstraint::primary_key) !=
                  all_columns[tbl_cols[i].getName()].getConstraints().end()) ||
                 (all_columns[tbl_cols[i].getName()].getConstraints().find(
                      ColumnConstraint::unique) !=
                  all_columns[tbl_cols[i].getName()].getConstraints().end()))) {
                auto dat = f[i].data;
                if (tbl_cols[i].getType() == DataType::varchar) {
                    dat = "null";
                }
                e.reset(new exc::constr::DuplicatedUnique(
                    name, table.getColumns()[i].getName(), dat));
                return;
            }
        }
    }

    if (!e) {
        cursor.insert(v_arr);
    }
}

void QueryManager::update(const Query& query,
                          std::unique_ptr<exc::Exception>& e,
                          std::ostream& out) {
    std::string name = query.getChildren()[NodeType::ident]->getName();
    auto table = Engine::show(name, e);
    if (table.getName().empty()) {
        e.reset(new exc::acc::TableNonexistent(name));
        return;
    }

    std::map<std::string, Column> all_columns;
    for (auto& c : table.getColumns()) {
        all_columns[c.getName()] = c;
    }

    auto idents =
        static_cast<IdentList*>(query.getChildren()[NodeType::ident_list])
            ->getIdents();
    std::set<std::string> col_set;
    for (auto& c : idents) {
        if (col_set.find(c->getName()) == col_set.end()) {
            col_set.insert(c->getName());
        } else {
            e.reset(new exc::RepeatColumn(c->getName()));
            return;
        }
    }

    auto constants =
        static_cast<ConstantList*>(query.getChildren()[NodeType::constant_list])
            ->getConstants();

    std::map<std::string, std::string> values;
    for (size_t i = 0; i < constants.size(); ++i) {
        if (Resolver::compareTypes(name, all_columns, idents[i], constants[i],
                                   e, true)) {
            if (all_columns[idents[i]->getName()].getType() ==
                    DataType::varchar &&
                all_columns[idents[i]->getName()].getN() <
                    static_cast<Constant*>(constants[i])->getValue().length()) {
                e.reset(new exc::DataTypeOversize(idents[i]->getName()));
                return;
            }
            values[idents[i]->getName()] =
                static_cast<Constant*>(constants[i])->getValue();
        } else {
            return;
        }
    }

    std::vector<int> unique_pos;
    int cnt = 0;
    for (auto& c : table.getColumns()) {
        if (c.getConstraints().find(ColumnConstraint::not_null) !=
                c.getConstraints().end() ||
            c.getConstraints().find(ColumnConstraint::primary_key) !=
                c.getConstraints().end() ||
            c.getConstraints().find(ColumnConstraint::unique) !=
                c.getConstraints().end()) {
            unique_pos.push_back(cnt);
        }
        ++cnt;
    }

    Cursor cursor(name);
    std::vector<std::vector<Value>> fetch_arr;
    std::vector<std::vector<Value>> ready_ftch;
    std::vector<std::vector<Value>> updated_records;
    while (cursor.next()) {
        auto ftch = cursor.fetch();
        fetch_arr.push_back(cursor.fetch());
        std::vector<Value> rec;
        for (int i = 0; i < table.getColumns().size(); ++i) {
            auto c = table.getColumns()[i];
            if (values.find(c.getName()) != values.end()) {
                Value v;
                v.is_null = false;
                auto tmp = values[c.getName()];
                if (tmp == "null") {
                    if (c.getType() == DataType::varchar) {
                        v.data = "";
                    } else {
                        v.data = "null";
                    }
                    v.is_null = true;
                } else {
                    v.data = values[c.getName()];
                }
                rec.push_back(v);
            } else {
                rec.push_back(ftch[i]);
            }
        }
        updated_records.push_back(rec);
        ready_ftch.push_back(ftch);
    }

    for (auto& f : fetch_arr) {
        for (auto& rec : updated_records) {
            for (auto& u : unique_pos) {
                if (f[u].data == rec[u].data) {
                    e.reset(new exc::constr::DuplicatedUnique(
                        name, table.getColumns()[u].getName(), f[u].data));
                    return;
                }
                if (rec[u].is_null &&
                    table.getColumns()[u].getConstraints().find(
                        ColumnConstraint::not_null) !=
                        table.getColumns()[u].getConstraints().end()) {
                    e.reset(new exc::constr::NullNotNull(
                        name, table.getColumns()[u].getName()));
                    return;
                }
            }
        }
    }

    cursor.reset();

    while (cursor.next()) {
        auto f = cursor.fetch();

        for (int k = 0; k < ready_ftch.size(); ++k) {
            for (int i = 0; i < ready_ftch[k].size(); ++i) {
                if (f[i].data != ready_ftch[k][i].data) {
                    continue;
                }
                std::map<std::string, std::string> m =
                    mapFromFetch(table.getColumns(), f);
                auto root = static_cast<Expression*>(
                    query.getChildren()[NodeType::expression]);
                std::string resp =
                    Resolver::resolve(name, all_columns, root, m, e);
                if (e) {
                    return;
                }

                if (resp != "0") {
                    cursor.update(updated_records[k]);
                }
            }
        }
    }
}

void QueryManager::remove(const Query& query,
                          std::unique_ptr<exc::Exception>& e,
                          std::ostream& out) {
    auto name = query.getChildren()[NodeType::ident]->getName();
    auto table = Engine::show(name, e);
    if (table.getName().empty()) {
        e.reset(new exc::acc::TableNonexistent(name));
        return;
    }

    std::map<std::string, Column> all_columns;
    for (auto& c : table.getColumns()) {
        all_columns[c.getName()] = c;
    }

    Cursor cursor(name);
    while (cursor.next()) {
        auto ftch = cursor.fetch();
        std::map<std::string, std::string> m =
            mapFromFetch(table.getColumns(), ftch);
        auto root =
            static_cast<Expression*>(query.getChildren()[NodeType::expression]);
        std::string resp = Resolver::resolve(name, all_columns, root, m, e);
        if (e) {
            return;
        }
        if (resp != "0") {
            cursor.remove();
        }
    }
}

std::map<std::string, std::string> QueryManager::mapFromFetch(
    const std::vector<Column>& cols, std::vector<Value> ftch) {
    std::map<std::string, std::string> m;
    int counter = 0;
    for (auto& k : cols) {
        if (ftch[counter].is_null) {
            if (k.getType() == DataType::varchar) {
                m[k.getName()] = "";
            } else {
                m[k.getName()] = "null";
            }
        } else {
            m[k.getName()] = ftch[counter].data;
        }

        ++counter;
    }
    return m;
}
