#include "Engine.h"
#include "../Core/Exception.h"

std::string Engine::getPathToTable(const std::string& name) {
    return "DataBD/" + name;
}

std::string Engine::getPathToTableMeta(const std::string& name) {
    return getPathToTable(name) + "_meta";
}

void Engine::create(const Table& table, std::unique_ptr<exc::Exception>& e) {
    if (exists(table.getName())) {
        e.reset(new exc::cr_table::TableName(table.getName()));
        return;
    }

    rapidjson::Document d(rapidjson::kObjectType);
    d.AddMember("name", table.getName(), d.GetAllocator());

    rapidjson::Value columns(rapidjson::kArrayType);
    for (const auto& column : table.getColumns()) {
        rapidjson::Value column_value(rapidjson::kObjectType);
        column_value.AddMember("name", column.getName(),d.GetAllocator());
        column_value.AddMember("type", static_cast<unsigned int>(column.getType()), d.GetAllocator());

        rapidjson::Value constraints(rapidjson::kArrayType);
        for (const auto& constraint : column.getConstraint()) {
            constraints.PushBack(static_cast<unsigned int>(constraint), d.GetAllocator());
        }
        column_value.AddMember("constraints", constraints, d.GetAllocator());

        columns.PushBack(column_value, d.GetAllocator());
    }
    d.AddMember("columns", columns, d.GetAllocator());

    try {
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        d.Accept(writer);

        std::ofstream metafile(getPathToTableMeta(table.getName()));
        metafile << buffer.GetString();

        std::ofstream(getPathToTable(table.getName()));
    }
    catch (std::bad_alloc& bad_alloc) {
        e.reset(new exc::OutOfMemory());
        std::remove(getPathToTable(table.getName()).c_str());
        std::remove(getPathToTableMeta(table.getName()).c_str());
    }
}

Table Engine::show(const std::string& name, std::unique_ptr<exc::Exception>& e) {
    if (!exists(name)) {
        e.reset(new exc::acc::TableNonexistent(name));
        return Table();
    }

    std::ifstream metafile(getPathToTableMeta(name));
    std::stringstream sstream;
    sstream << metafile.rdbuf();

    rapidjson::Document d;
    d.Parse(sstream.str());

    Table table;
    table.setName(d["name"].GetString());

    const rapidjson::Value& columns_value = d["columns"];
    for (auto& column_value : columns_value.GetArray()) {
        std::set<ColumnConstraint> constraints;

        const rapidjson::Value& constraints_value = column_value["constraints"];
        for (auto& constraint_value : constraints_value.GetArray()) {
            constraints.insert(static_cast<ColumnConstraint >(constraint_value.GetInt()));
        }

        std::unique_ptr<exc::Exception> e;
        table.addColumn(Column(
                column_value["name"].GetString(),
                static_cast<DataType>(column_value["type"].GetInt()), e,
                constraints
        ), e);
    }

    return table;
}

std::string Engine::showCreate(const std::string& name, std::unique_ptr<exc::Exception>& e) {
    if (!exists(name)) {
        e.reset(new exc::acc::TableNonexistent(name));
        return std::string();
    }
    Table table = show(name, e);
    std::string query("CREATE TABLE " + table.getName() + "(");

    for (unsigned int i = 0; i < table.getColumns().size(); ++i) {
        Column column = table.getColumns()[i];
        query.append("\n    " + column.getName() + " " + DataType2String(column.getType()));

        for (const auto& constraint : column.getConstraint()) {
            query.append(" " + ColumnConstraint2String(constraint));
        }

        if (i != table.getColumns().size() - 1) {
            query.append(",");
        }
    }

    query.append("\n);");
    return query;
}

void Engine::drop(const std::string& name, std::unique_ptr<exc::Exception>& e) {
    if (!exists(name)) {
        e.reset(new exc::acc::TableNonexistent(name));
        return;
    }
    std::remove(getPathToTable(name).c_str());
    std::remove(getPathToTableMeta(name).c_str());
}

bool Engine::exists(const std::string& name) {
    return static_cast<bool>(std::ifstream(getPathToTableMeta(name)));
}

void Engine::load(const std::string& name, std::unique_ptr<exc::Exception>& e) {
}

void Engine::commit(const std::string& name, std::unique_ptr<exc::Exception>& e) {
}

void Engine::free(const std::string& name, std::unique_ptr<exc::Exception>& e) {
}