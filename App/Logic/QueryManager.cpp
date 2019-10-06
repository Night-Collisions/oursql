#include "QueryManager.h"
#include "../../App/Engine/Field.h"
#include "../../App/Engine/Table.h"
#include "../Engine/Engine.h"
#include "Parser/Nodes/Command.h"
#include "Parser/Nodes/Ident.h"
#include "Parser/Nodes/VarList.h"

void QueryManager::execute(const Query& query) {
    switch (static_cast<Command*>(query.getChildren()[0])->getCommandType()) {
        case CommandType::create_table:
            createTable(query);
            break;
    }
}

void QueryManager::createTable(const Query& query) {
    std::string name = static_cast<Ident*>(query.getChildren()[1])->getName();

    std::vector<Field> columns;
    auto vars = static_cast<VarList*>(query.getChildren()[2])->getVars();
    for (auto& v : vars) {
        std::string col_name = v->getName();
        DataType type = v->getType();
        auto constraints = v->getConstraints();

        Field f(col_name, type, constraints);
        columns.emplace_back(f);
    }

    Table table(name, columns);

    create(table);
}
