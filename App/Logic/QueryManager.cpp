#include "QueryManager.h"

#include <memory>

#include "../../App/Engine/Table.h"
#include "../Engine/Column.h"
#include "../Engine/Engine.h"
#include "Parser/Nodes/Command.h"
#include "Parser/Nodes/Ident.h"
#include "Parser/Nodes/VarList.h"

#include "../../App/Core/Exception.h"

void QueryManager::execute(const Query& query, std::unique_ptr<exc::Exception>& e) {
    void (*const commandsActions[static_cast<unsigned int>(
        CommandType::Count)])(const Query& query, std::unique_ptr<exc::Exception>& e) = {
        [](const Query&, std::unique_ptr<exc::Exception>& e) {}, createTable, showCreateTable,
        dropTable};
    CommandType command =
        static_cast<Command*>(query.getChildren()[0])->getCommandType();
    if (command != CommandType::Count) {
        commandsActions[static_cast<unsigned int>(command)](query, e);
    }
}

void QueryManager::createTable(const Query& query, std::unique_ptr<exc::Exception>& e) {
    // TODO: накидать исключений
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
                e.reset(new exc::constr::RedundantConstraints(name, v->getName()));
                return;
            }
        }

        Column f(col_name, type, e, constr_set);
        columns.emplace_back(f);
    }

    Table table(name, columns);

    create(table);  // TODO: она что то возвращает, надо кинуть исключение
}

/*void QueryManager::checkConstraints(
    const std::set<ColumnConstraint>& constraint) {
    std::array<std::set<ColumnConstraint>,
               static_cast<unsigned int>(ColumnConstraint::Count)>
        incompatible = {std::set<ColumnConstraint>{},
                        std::set<ColumnConstraint>{},
                        std::set<ColumnConstraint>{}};
    for (const auto& i : constraint) {
        std::set<ColumnConstraint> buff;
        std::set_intersection(
            incompatible[static_cast<unsigned int>(i)].begin(),
            incompatible[static_cast<unsigned int>(i)].end(),
            constraint.begin(), constraint.end(),
            std::inserter(buff, buff.begin()));
        if (buff.size() > 0) {
            throw std::invalid_argument("Incompatible constraints");
        }
    }
}*/

void QueryManager::showCreateTable(const Query& query, std::unique_ptr<exc::Exception>& e) {
    if (query.getChildren().size() != 2) {
        // TODO: исключение, неправильно составлени команда, так как запрос
        // должен быть из двух частей
    }

    // TODO: print to output stream
    showCreate(static_cast<Ident*>(query.getChildren()[1])->getName());
}

void QueryManager::dropTable(const Query& query, std::unique_ptr<exc::Exception>& e) {}