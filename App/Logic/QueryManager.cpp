#include "QueryManager.h"

#include <memory>

#include "../Engine/Column.h"
#include "../Engine/Engine.h"
#include "Parser/Nodes/Ident.h"
#include "Parser/Nodes/VarList.h"

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
    for (auto& m : all_columns) {
        asterisk.insert(m.first);
    }
    ConditionChecker* checker = new ConditionChecker(true);
    auto doc = Engine::select(name, asterisk, *checker, e);
    auto vals = doc["values"].GetArray();

    std::vector<Expression*> col_expr;

    for (auto& c : cols_from_parser) {
        if (c->getNodeType() == NodeType::expression_unit) {
            col_expr.push_back(static_cast<Expression*>(
                query.getChildren()[NodeType::expression]));
        }
    }

    if (e != nullptr) {
        return;
    }

    for (auto& i : vals) {
        auto root =
            static_cast<Expression*>(query.getChildren()[NodeType::expression]);
        Resolver::resolve(name, all_columns, root, i, e);
        if (e) {
            return;
        }

        int expr_cnt = 1;
        std::string calc =
            (root) ? (static_cast<Constant*>(root->getConstant())->getValue())
                   : ("1");
        if (calc != "0") {
            for (auto& c : cols_from_parser) {
                if (c->getNodeType() == NodeType::expression_unit) {
                    auto expr = static_cast<Expression*>(c);
                    Resolver::resolve(name, all_columns, expr, i, e);
                    if (e) {
                        return;
                    }
                    out << "expression " + std::to_string(expr_cnt++) + ": " +
                               static_cast<Constant*>(expr->getConstant())
                                   ->getValue()
                        << std::endl;
                } else if (c->getName() == "*") {
                    for (auto& k : table.getColumns()) {
                        out << k.getName() + ": " + i[k.getName()].GetString()
                            << std::endl;
                    }
                } else {
                    out << c->getName() + ": " + i[c->getName()].GetString()
                        << std::endl;
                }
            }
        }
    }

    delete checker;
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
    if (!idents.empty()) {
        std::set<std::string> col_set;
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

    std::unordered_map<std::string, std::string> values;

    for (size_t i = 0; i < constants.size(); ++i) {
        if (Resolver::compareTypes(name, all_columns, idents[i], constants[i],
                                   e, true)) {
            values[idents[i]->getName()] =
                static_cast<Constant*>(constants[i])->getValue();
        } else {
            return;
        }
    }

    Engine::insert(name, values, e);
}

void QueryManager::update(const Query& query,
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

    auto idents =
        static_cast<IdentList*>(query.getChildren()[NodeType::ident_list])
            ->getIdents();
    auto constants =
        static_cast<ConstantList*>(query.getChildren()[NodeType::constant_list])
            ->getConstants();

    ConditionChecker* c = new ConditionChecker(true);

    std::unordered_map<std::string, std::string> values;

    for (size_t i = 0; i < constants.size(); ++i) {
        if (Resolver::compareTypes(name, all_columns, idents[i], constants[i],
                                   e, true)) {
            values[idents[i]->getName()] =
                static_cast<Constant*>(constants[i])->getValue();
        } else {
            return;
        }
    }

    Engine::update(name, values, *c, e);

    delete c;
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

    ConditionChecker* checker = nullptr;

    std::map<std::string, Column> all_columns;
    for (auto& c : table.getColumns()) {
        all_columns[c.getName()] = c;
    }

    auto rel = static_cast<Relation*>(query.getChildren()[NodeType::relation]);
    if (rel) {
        auto left = rel->getLeft();
        auto right = rel->getRight();
        auto op = rel->getOperator();
        DataType comp_type;
        std::string left_value;
        std::string right_value;

        if (Resolver::compareTypes(name, all_columns, left, right, e, false)) {
            setValue(left, left_value);
            setValue(right, right_value);
            if (left->getNodeType() == NodeType::ident) {
                comp_type = all_columns[left->getName()].getType();
            } else {
                comp_type = static_cast<Constant*>(left)->getDataType();
            }
        } else {
            return;
        }

        checker =
            new ConditionChecker(left_value, right_value, left->getNodeType(),
                                 right->getNodeType(), op, comp_type);
    }

    if (checker == nullptr) {
        checker = new ConditionChecker(true);
    }

    Engine::remove(name, *checker, e);

    delete checker;
}

void QueryManager::setValue(Node* nod, std::string& value) {
    if (nod->getNodeType() == NodeType::ident) {
        value = nod->getName();
    } else {
        value = static_cast<Constant*>(nod)->getValue();
    }
}