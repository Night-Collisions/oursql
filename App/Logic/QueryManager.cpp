#include "QueryManager.h"
#include "../../App/Engine/Table.h"
#include "../Engine/Column.h"
#include "../Engine/Engine.h"
#include "Parser/Nodes/Command.h"
#include "Parser/Nodes/Ident.h"
#include "Parser/Nodes/VarList.h"

void QueryManager::execute(const Query& query) {
    switch (static_cast<Command*>(query.getChildren()[0])->getCommandType()) {
        case CommandType::create_table:
            createTable(query);
            break;
        case CommandType::show_create_table:
            showCreateTable(query);
            break;
        case CommandType::drop_table:
            dropTable(query);
            break;
    }
}

void QueryManager::createTable(const Query& query) {
    //TODO: накидать исключений
    std::string name = static_cast<Ident*>(query.getChildren()[1])->getName();

    std::vector<Column> columns;
    auto vars = static_cast<VarList*>(query.getChildren()[2])->getVars();
    for (auto& v : vars) {
        std::string col_name = v->getName();
        DataType type = v->getType();
        auto constraints = v->getConstraints();
        checkConstraints(constraints); //TODO: внутри кидать новые исключения

        Column f(col_name, type, constraints);
        columns.emplace_back(f);
    }

    Table table(name, columns);

    create(table); //TODO: она что то возвращает, надо кинуть исключение
}

void QueryManager::checkConstraints(const std::set<ColumnConstraint>& constraint) {
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