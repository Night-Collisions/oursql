#include "Engine.h"

static inline std::string getPathToTable(const std::string& name) {
    return name;
}

static inline std::string getPathToTableMeta(const std::string& name) {
    return getPathToTable(name) + "_meta";
}

bool create(const Table& table) {
    if (exists(table.getName())) {
        return true;
    }

    rapidjson::Document d(rapidjson::kObjectType);
    d.AddMember("name", table.getName(), d.GetAllocator());

    rapidjson::Value fields(rapidjson::kArrayType);
    for (const auto& field : table.getFields()) {
        rapidjson::Value field_value(rapidjson::kObjectType);
        field_value.AddMember("name", field.getName(),d.GetAllocator());
        field_value.AddMember("type", static_cast<unsigned int>(field.getType()), d.GetAllocator());

        rapidjson::Value constraints(rapidjson::kArrayType);
        for (const auto& constraint : field.getConstraint()) {
            constraints.PushBack(static_cast<unsigned int>(constraint), d.GetAllocator());
        }
        field_value.AddMember("constraints", constraints, d.GetAllocator());

        fields.PushBack(field_value, d.GetAllocator());
    }
    d.AddMember("fields", fields, d.GetAllocator());

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    d.Accept(writer);

    std::ofstream metafile(getPathToTableMeta(table.getName()));
    metafile << buffer.GetString();

    std::ofstream(getPathToTable(table.getName()));
    return false;
}

Table show(const std::string& name) {
    if (!exists(name)) {
        return Table();
    }

    std::ifstream metafile(getPathToTableMeta(name));
    std::stringstream sstream;
    sstream << metafile.rdbuf();

    rapidjson::Document d;
    d.Parse(sstream.str());

    Table table;
    table.setName(d["name"].GetString());

    const rapidjson::Value& fields_value = d["fields"];
    for (auto& field_value : fields_value.GetArray()) {
        std::set<FieldConstraint> constraints;

        const rapidjson::Value& constraints_value = field_value["constraints"];
        for (auto& constraint_value : constraints_value.GetArray()) {
            constraints.insert(static_cast<FieldConstraint >(constraint_value.GetInt()));
        }

        table.addField(Field(
                field_value["name"].GetString(),
                static_cast<DataType>(field_value["type"].GetInt()),
                constraints
        ));
    }

    return table;
}

std::string showCreate(const std::string& name) {
    if (!exists(name)) {
        return std::string();
    }
    Table table = show(name);
    std::string query("CREATE TABLE " + table.getName() + "(");

    for (unsigned int i = 0; i < table.getFields().size(); ++i) {
        Field field = table.getFields()[i];
        query.append("\n    " + field.getName() + " " + DataType2String(field.getType()));

        for (const auto& constraint : field.getConstraint()) {
            query.append(" " + FieldConstraint2String(constraint));
        }

        if (i != table.getFields().size() - 1) {
            query.append(",");
        }
    }

    query.append("\n);");
    return query;
}

bool drop(const std::string& name) {
    if (!exists(name)) {
        return true;
    }
    std::remove(getPathToTable(name).c_str());
    std::remove(getPathToTableMeta(name).c_str());
}

bool exists(const std::string& name) {
    return static_cast<bool>(std::ifstream(getPathToTableMeta(name)));
}