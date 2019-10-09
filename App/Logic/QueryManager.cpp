#include "QueryManager.h"

#include <memory>

#include "../../App/Engine/Table.h"
#include "../Engine/Column.h"
#include "../Engine/Engine.h"
#include "Parser/Nodes/Command.h"
#include "Parser/Nodes/Ident.h"
#include "Parser/Nodes/VarList.h"

#include "../../App/Core/Exception.h"
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
        createTable, showCreateTable, dropTable, select};
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
    if (e != nullptr)
        return;

    if (create(table)) {
        e.reset(new exc::cr_table::RepeatTableName(name));
    }
}

void QueryManager::showCreateTable(const Query& query,
                                   std::unique_ptr<exc::Exception>& e,
                                   std::ostream& out) {
    // TODO: print to output stream
    auto name = static_cast<Ident*>(query.getChildren()[1])->getName();
    auto res = showCreate(name);
    if (res.empty()) {
        e.reset(new exc::acc::TableNonexistent(name));
    } else {
        out << res << std::endl;
    }
}

void QueryManager::dropTable(const Query& query,
                             std::unique_ptr<exc::Exception>& e,
                             std::ostream& out) {
    auto name = static_cast<Ident*>(query.getChildren()[1])->getName();
    if (drop(name)) {
        e.reset(new exc::acc::TableNonexistent(name));
    }
}
void QueryManager::select(const Query& query,
                          std::unique_ptr<exc::Exception>& e,
                          std::ostream& out) {
    auto name = static_cast<Ident*>(query.getChildren()[1])->getName();
    auto table = show(name);

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
            std::copy(existing_cols.begin(), existing_cols.end(), ready_cols.begin());
        } else if (col_set.find((c.getName())) == col_set.end()) {
            e.reset(new exc::acc::ColumnNonexistent(c.getName(), name));
            return;
        }
        ready_cols.push_back(c.getName());
    }

   //WARNING!!! THIS FUNCTION IS NOT FINISHED

}
