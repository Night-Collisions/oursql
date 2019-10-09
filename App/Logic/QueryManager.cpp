#include "QueryManager.h"

#include <memory>

#include "../../App/Engine/Table.h"
#include "../Engine/Column.h"
#include "../Engine/Engine.h"
#include "Parser/Nodes/Command.h"
#include "Parser/Nodes/Ident.h"
#include "Parser/Nodes/VarList.h"

#include "../../App/Core/Exception.h"
#include "Conditions/ConditionChecker.h"
#include "Parser/Nodes/Constant.h"
#include "Parser/Nodes/Relation.h"
#include "Parser/Nodes/SelectList.h"

void QueryManager::execute(const Query& query,
                           std::unique_ptr<exc::Exception>& e,
                           std::ostream& out) {
    void (*const
              commandsActions[static_cast<unsigned int>(CommandType::Count)])(
        const Query& query, std::unique_ptr<exc::Exception>& e,
        std::ostream& out) = {
        [](const Query&, std::unique_ptr<exc::Exception>& e,
           std::ostream& out) {},
        createTable, showCreateTable, dropTable, select, insert};
    CommandType command =
        static_cast<Command*>(query.getChildren()[0])->getCommandType();
    if (command != CommandType::Count) {
        commandsActions[static_cast<unsigned int>(command)](query, e, out);
    }
}

void QueryManager::createTable(const Query& query,
                               std::unique_ptr<exc::Exception>& e,
                               std::ostream& out) {
    e.reset(nullptr);

    std::string name = static_cast<Ident*>(query.getChildren()[1])->getName();

    std::vector<Column> columns;
    auto vars = static_cast<VarList*>(query.getChildren()[2])->getVars();
    for (auto& v : vars) {
        std::string col_name = v->getName();
        DataType type = v->getType();
        auto constr_vector = v->getConstraints();

        std::set<ColumnConstraint> constr_set;

        for (auto& c : constr_vector) {
            if (constr_set.find(c) == constr_set.end()) {
                constr_set.insert(c);
            } else {
                e.reset(
                    new exc::constr::RedundantConstraints(name, v->getName()));
                return;
            }
        }

        Column f(col_name, type, e, constr_set);
        columns.emplace_back(f);
    }

    Table table(name, columns, e);
    if (e != nullptr) return;

    Engine::create(table, e);
}

void QueryManager::showCreateTable(const Query& query,
                                   std::unique_ptr<exc::Exception>& e,
                                   std::ostream& out) {
    // TODO: print to output stream
    auto name = static_cast<Ident*>(query.getChildren()[1])->getName();
    auto res = Engine::showCreate(name, e);
    out << res << std::endl;
}

void QueryManager::dropTable(const Query& query,
                             std::unique_ptr<exc::Exception>& e,
                             std::ostream& out) {
    auto name = static_cast<Ident*>(query.getChildren()[1])->getName();
    Engine::drop(name, e);
}
void QueryManager::select(const Query& query,
                          std::unique_ptr<exc::Exception>& e,
                          std::ostream& out) {
    auto name = static_cast<Ident*>(query.getChildren()[1])->getName();
    auto table = Engine::show(name, e);

    std::vector<std::string> existing_cols;
    std::set<std::string> col_set;
    for (auto& c : table.getColumns()) {
        existing_cols.push_back(c.getName());
        col_set.insert(c.getName());
    }

    auto cols_from_parser =
        static_cast<SelectList*>(query.getChildren()[2])->getList();

    std::vector<std::string> ready_cols;
    for (auto& c : cols_from_parser) {
        if (c.getName() == "*") {
            ready_cols.resize(cols_from_parser.size());
            std::copy(existing_cols.begin(), existing_cols.end(),
                      ready_cols.begin());
        } else if (col_set.find((c.getName())) == col_set.end()) {
            e.reset(new exc::acc::ColumnNonexistent(c.getName(), name));
            return;
        } else {
            ready_cols.push_back(c.getName());
        }
    }

    auto rel = static_cast<Relation*>(query.getChildren()[3]);
    auto left = rel->getLeft();
    auto right = rel->getRight();
    auto op = rel->getRelation();
    DataType left_type;
    DataType right_type;
    std::string left_value;
    std::string right_value;

    if (left->getNodeType() == NodeType::id) {
        for (auto& c : table.getColumns()) {
            if (static_cast<Ident*>(left)->getName() == c.getName()) {
                left_type = c.getType();
                left_value = static_cast<Constant*>(left)->getValue();
            }
        }
    } else {
        left_type = static_cast<Constant*>(left)->getDataType();
        left_value = static_cast<Constant*>(left)->getValue();
    }

    if (right->getNodeType() == NodeType::id) {
        for (auto& c : table.getColumns()) {
            if (static_cast<Ident*>(right)->getName() == c.getName()) {
                right_type = c.getType();
                right_value = static_cast<Constant*>(right)->getValue();
            }
        }
    } else {
        right_type = static_cast<Constant*>(right)->getDataType();
        right_value = static_cast<Constant*>(right)->getValue();
    }

    if (left_type != right_type) {
        e.reset(new exc::CompareDataTypeMismatch(left_type, right_type));
        return;
    }

    ConditionChecker c(left_value, right_value, left->getNodeType(),
                       right->getNodeType(), rel->getRelation(), left_type);

    std::set<std::string> cols_to_engine;

    for (auto& c : ready_cols) {
        cols_to_engine.insert(c);
    }

    auto doc = Engine::select(name, cols_to_engine, c, e);

    //todo
}
void QueryManager::insert(const Query& query,
                          std::unique_ptr<exc::Exception>& e,
                          std::ostream& out) {

}
