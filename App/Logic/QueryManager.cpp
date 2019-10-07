#include "QueryManager.h"

#include "../../App/Engine/Table.h"
#include "../Engine/Column.h"
#include "../Engine/Engine.h"
#include "Parser/Nodes/Command.h"
#include "Parser/Nodes/Ident.h"
#include "Parser/Nodes/VarList.h"

void QueryManager::execute(const Query& query) {
    void (* const commandsExtions[static_cast<unsigned int>(CommandType::Count)])(
        const Query& query) = {
        [](const Query&) {},
        createTable,
        showCreateTable,
        dropTable
    };
    CommandType command = static_cast<Command*>(query.getChildren()[0])->getCommandType();
    if (command != CommandType::Count) {
        commandsExtions[static_cast<unsigned int>(command)](query);
    }
}

void QueryManager::createTable(const Query& query) {
    // TODO: накидать исключений
    std::string name = static_cast<Ident*>(query.getChildren()[1])->getName();

    std::vector<Column> columns;
    auto vars = static_cast<VarList*>(query.getChildren()[2])->getVars();
    for (auto& v : vars) {
        std::string col_name = v->getName();
        DataType type = v->getType();
        auto constraints = v->getConstraints();
        checkConstraints(constraints);  // TODO: внутри кидать новые исключения

        Column f(col_name, type, constraints);
        columns.emplace_back(f);
    }

    Table table(name, columns);

    create(table);  // TODO: она что то возвращает, надо кинуть исключение
}

void QueryManager::checkConstraints(
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
}