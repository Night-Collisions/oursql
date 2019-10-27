#include "QueryManager.h"
#include <iostream>
#include <memory>

#include "../Engine/Column.h"
#include "../Engine/Cursor.h"
#include "../Engine/Engine.h"
#include "Parser/Nodes/Ident.h"
#include "Parser/Nodes/VarList.h"

#include "Parser/ExpressionParser/Resolver.h"
#include "Parser/Nodes/ConstantList.h"
#include "Parser/Nodes/IdentList.h"
#include "Parser/Nodes/RelExpr.h"
#include "Parser/Nodes/SelectList.h"
#include "Parser/RelationalOperationsParser/Join.h"

std::array<rel_func, static_cast<unsigned int>(RelOperNodeType::Count)>
    QueryManager::relational_oper_ = {Join::makeJoin, Join::makeJoin};

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

void printSelect(const Table& table, t_column_infos column_infos,
                 std::vector<Node*> cols_from_parser, t_record_infos record,
                 std::unique_ptr<exc::Exception>& e, std::ostream& out) {
    std::string response;
    int expr_cnt = 1;
    out << "=======\n";
    for (auto& c : cols_from_parser) {
        auto expr = static_cast<Expression*>(c);
        std::string prefix =
            (table.getName().empty()) ? ("") : (table.getName() + ".");
        if (expr->getConstant()->getName() == "*") {
            for (auto& k : table.getColumns()) {
                out << prefix + k.getName() + ": " +
                           record[table.getName()][k.getName()]
                    << std::endl;
            }
            continue;
        }
        response = Resolver::resolve(table.getName(), table.getName(),
                                     column_infos, expr, record, e);
        std::string colname = expr->getConstant()->getName();
        if (colname.empty()) {
            colname = "expression " + std::to_string(expr_cnt++);
        } else {
            auto id = static_cast<Ident*>(expr->getConstant());
            colname = (id->getTableName().empty()) ? ("")
                                                   : (id->getTableName() + ".");
            colname += id->getName();
        }
        if (e) {
            return;
        }
        out << colname + ": " + response << std::endl;
    }
}

std::map<std::string, Column> getColumnMap(const Table& t) {
    std::map<std::string, Column> all_columns;
    for (auto& c : t.getColumns()) {
        all_columns[c.getName()] = c;
    }

    return all_columns;
}

void QueryManager::select(const Query& query,
                          std::unique_ptr<exc::Exception>& e,
                          std::ostream& out) {
    auto children = query.getChildren();

    Table resolvedTable;
    t_column_infos column_info;
    std::vector<Node*> cols_from_parser;

    if (children.find(NodeType::relational_oper_expr) != children.end()) {
        auto root =
            static_cast<RelExpr*>(children[NodeType::relational_oper_expr]);
        resolvedTable = resolveRelationalOperTree(root, e);
        if (e) {
            return;
        }

    } else {
        auto name = children[NodeType::ident]->getName();
        resolvedTable = getFilledTable(name, e);

        if (e) {
            return;
        }
    }

    column_info[resolvedTable.getName()] = getColumnMap(resolvedTable);

    cols_from_parser =
        static_cast<SelectList*>(children[NodeType::select_list])->getList();

    for (auto& c : cols_from_parser) {
        std::string colname;
        std::string tablename;
        auto expr = static_cast<Expression*>(c);
        if (expr->getConstant()->getName() == "*") {
            continue;
        }
        auto node = expr->getConstant();
        if (node->getNodeType() == NodeType::ident) {
            auto id = static_cast<Ident*>(node);
            /*            tablename = (id->getTableName().empty()) ?
               (resolvedTable.getName()) : (id->getTableName());*/
            tablename = resolvedTable.getName();
            colname = node->getName();
            if (tablename.empty()) {
                /*                tablename = (id->getTableName().empty())
                                                ? (resolvedTable.getName())
                                                : (id->getTableName());*/
                colname = (id->getTableName().empty())
                              ? (node->getName())
                              : (id->getTableName() + "." + node->getName());
            }

            id->setTableName(tablename);
            id->setName(colname);
        } else {
            colname = node->getName();
        }

        if (node->getNodeType() == NodeType::ident &&
            column_info[resolvedTable.getName()].find(colname) ==
                column_info[resolvedTable.getName()].end()) {
            e.reset(new exc::acc::ColumnNonexistent(colname,
                                                    resolvedTable.getName()));
            return;
        }
    }

    auto records = resolvedTable.getRecords();
    t_record_infos record_info;
    for (int i = 0; i < records.size(); ++i) {
        record_info[resolvedTable.getName()] =
            Resolver::getRecord(resolvedTable.getColumns(), records[i]);

        auto root = static_cast<Expression*>(children[NodeType::expression]);
        std::string response =
            Resolver::resolve(resolvedTable.getName(), resolvedTable.getName(),
                              column_info, root, record_info, e);
        if (e) {
            return;
        }
        if (response != "0") {
            printSelect(resolvedTable, column_info, cols_from_parser,
                        record_info, e, out);
            if (e) {
                return;
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

    std::map<std::string, std::map<std::string, Column>> column_info;
    for (auto& c : table.getColumns()) {
        column_info[table.getName()][c.getName()] = c;
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
        if (Resolver::compareTypes(name, name, column_info, idents[i],
                                   constants[i], e, CompareCondition::assign,
                                   "=")) {
            if (column_info[name][idents[i]->getName()].getType() ==
                    DataType::varchar &&
                column_info[name][idents[i]->getName()].getN() <
                    static_cast<Constant*>(constants[i])->getValue().length()) {
                e.reset(new exc::DataTypeOversize(idents[i]->getName()));
                return;
            }
            if (static_cast<Constant*>(constants[i])->getDataType() !=
                DataType::null_) {
                auto val = static_cast<Constant*>(constants[i])->getValue();
                if (column_info[name][idents[i]->getName()].getType() ==
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
                ((column_info[name][tbl_cols[i].getName()]
                      .getConstraints()
                      .find(ColumnConstraint::primary_key) !=
                  column_info[name][tbl_cols[i].getName()]
                      .getConstraints()
                      .end()) ||
                 (column_info[name][tbl_cols[i].getName()]
                      .getConstraints()
                      .find(ColumnConstraint::unique) !=
                  column_info[name][tbl_cols[i].getName()]
                      .getConstraints()
                      .end()))) {
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

    std::map<std::string, std::map<std::string, Column>> column_info;
    for (auto& c : table.getColumns()) {
        column_info[table.getName()][c.getName()] = c;
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
        if (Resolver::compareTypes(name, name, column_info, idents[i],
                                   constants[i], e, CompareCondition::assign,
                                   "=")) {
            if (column_info[name][idents[i]->getName()].getType() ==
                    DataType::varchar &&
                column_info[name][idents[i]->getName()].getN() <
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
                std::map<std::string, std::map<std::string, std::string>>
                    record;
                std::map<std::string, std::string> m =
                    Resolver::getRecord(table.getColumns(), f);
                record[name] = m;

                auto root = static_cast<Expression*>(
                    query.getChildren()[NodeType::expression]);
                std::string resp =
                    Resolver::resolve(name, name, column_info, root, record, e);
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

    std::map<std::string, std::map<std::string, Column>> column_info;
    for (auto& c : table.getColumns()) {
        column_info[table.getName()][c.getName()] = c;
    }

    Cursor cursor(name);
    while (cursor.next()) {
        auto ftch = cursor.fetch();
        std::map<std::string, std::map<std::string, std::string>> record;
        std::map<std::string, std::string> m =
            Resolver::getRecord(table.getColumns(), ftch);
        record[name] = m;
        auto root =
            static_cast<Expression*>(query.getChildren()[NodeType::expression]);
        std::string resp =
            Resolver::resolve(name, name, column_info, root, record, e);
        if (e) {
            return;
        }
        if (resp != "0") {
            cursor.remove();
        }
    }
}

Table QueryManager::resolveRelationalOperTree(
    RelExpr* root, std::unique_ptr<exc::Exception>& e) {
    if (e) {
        return Table();
    }

    if (root && root->childs()[0] && root->childs()[1]) {
        auto child1 = root->childs()[0];
        auto child2 = root->childs()[1];

        Table table1;
        Table table2;

        if (child1->getRelOperType() == RelOperNodeType::table_ident) {
            table1 = getFilledTable(child1->getName(), e);
            if (e) {
                return Table();
            }
        } else {
            table1 = resolveRelationalOperTree(child1, e);
            if (e) {
                return Table();
            }
        }

        if (child2->getRelOperType() == RelOperNodeType::table_ident) {
            table2 = getFilledTable(child2->getName(), e);
            if (e) {
                return Table();
            }
        } else {
            table2 = resolveRelationalOperTree(child2, e);
            if (e) {
                return Table();
            }
        }

        Table res_table =
            relational_oper_[static_cast<unsigned int>(root->getRelOperType())](
                table1, table2, root->getOnExpr(), e, root->getRelOperType());

        res_table.setName(root->getAlias());

        if (e) {
            return Table();
        }

        return res_table;
    }
}

Table QueryManager::getFilledTable(const std::string& name,
                                   std::unique_ptr<exc::Exception>& e) {
    auto table = Engine::show(name, e);
    if (e) {
        return Table();
    }
    table.setName(name);
    auto cursor = Cursor(name);

    while (cursor.next()) {
        table.addRecord(cursor.fetch(), e);
        if (e) {
            return Table();
        }
    }

    return table;
}
