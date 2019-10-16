#include "Engine.h"
#include "../Core/Exception.h"

// Metafile:
// reserved: 1 byte;
// table name: 128 bytes (with '\0');
// columns count: 1 byte;
// columns: reserved: 1 byte, type: 1 byte, constraints: 1 byte, column name: 128 bytes (with '\0').

std::string Engine::getPathToTable(const std::string& table_name) {
    return "DataBD/" + table_name;
}

std::string Engine::getPathToTableMeta(const std::string& table_name) {
    return getPathToTable(table_name) + "_meta";
}

void Engine::create(const Table& table, std::unique_ptr<exc::Exception>& e) {
    if (exists(table.getName())) {
        e.reset(new exc::cr_table::RepeatTableName(table.getName()));
        return;
    }

    try {
        std::ofstream metafile(getPathToTableMeta(table.getName()), std::ios::binary);
        unsigned char reserved = 0;
        metafile << reserved;
        char table_name[kTableNameLength_] = {0};
        std::memcpy(table_name, table.getName().c_str(), table.getName().size());
        metafile.write(table_name, kTableNameLength_);
        unsigned char columns_count = table.getColumns().size();
        metafile << columns_count;

        for (const auto& column : table.getColumns()) {
            unsigned char reserved = 0;
            metafile << reserved;
            unsigned char type = static_cast<unsigned char>(column.getType());
            metafile << type;
            unsigned char constraints = column.getConstraints();
            metafile << constraints;
            char column_name[kColumnNameLength_] = {0};
            std::memcpy(column_name, column.getName().c_str(), column.getName().size());
            metafile.write(column_name, kColumnNameLength_);
        }

        std::ofstream(getPathToTable(table.getName()));
    } catch (std::bad_alloc& bad_alloc) {
        e.reset(new exc::OutOfMemory());
        std::remove(getPathToTable(table.getName()).c_str());
        std::remove(getPathToTableMeta(table.getName()).c_str());
    }
}

Table Engine::show(const std::string& table_name, std::unique_ptr<exc::Exception>& e) {
    if (!exists(table_name)) {
        e.reset(new exc::acc::TableNonexistent(table_name));
        return Table();
    }

    std::ifstream metafile(getPathToTableMeta(table_name), std::ios::binary);
    Table table;

    unsigned char reserved;
    metafile >> reserved;
    char table_name[kTableNameLength_];
    metafile.read(table_name, kTableNameLength_);
    table.setName(table_name);
    unsigned char columns_count;
    metafile >> columns_count;

    for (int i = 0; i < columns_count; ++i) {
        unsigned char reserved;
        metafile >> reserved;
        unsigned char type;
        metafile >> type;
        unsigned char constraints;
        metafile >> constraints;
        char column_name[kTableNameLength_];
        metafile.read(column_name, kColumnNameLength_);
        std::unique_ptr<exc::Exception> e;
        table.addColumn(Column(column_name, static_cast<DataType>(type), e, constraints), e);
    }

    return table;
}

std::string Engine::showCreate(const std::string& table_name, std::unique_ptr<exc::Exception>& e) {
    if (!exists(table_name)) {
        e.reset(new exc::acc::TableNonexistent(table_name));
        return std::string();
    }
    Table table = show(table_name, e);
    std::string query("CREATE TABLE " + table.getName() + "(");

    for (unsigned int i = 0; i < table.getColumns().size(); ++i) {
        Column column = table.getColumns()[i];
        query.append("\n    " + column.getName() + " " +
                     DataType2String(column.getType()));

        unsigned char mask = 1;
        while (mask != 0) {
            unsigned char constraint = mask & column.getConstraint();
            if (constraint != 0) {
                query.append(" " + ColumnConstraint2String(constraint));
            }
            mask <<= 1u;
        }

        if (i != table.getColumns().size() - 1) {
            query.append(",");
        }
    }

    query.append("\n);");
    return query;
}

void Engine::drop(const std::string& table_name, std::unique_ptr<exc::Exception>& e) {
    if (!exists(table_name)) {
        e.reset(new exc::acc::TableNonexistent(table_name));
        return;
    }
    std::remove(getPathToTable(table_name).c_str());
    std::remove(getPathToTableMeta(table_name).c_str());
}

bool Engine::exists(const std::string& table_name) {
    return static_cast<bool>(std::ifstream(getPathToTableMeta(table_name)));
}

void Engine::freeMemory(const std::string& table_name) {
    std::unique_ptr<exc::Exception> e;
    Table table = show(table_name, e);
    std::string salt("afhh2lhrlfjlsdjfnh34232432gfg");
    std::fstream old_file(getPathToTable(table_name), std::ios::binary | std::ios::in);
    std::fstream new_file(getPathToTable(table_name) + salt, std::ios::binary | std::ios::out);
    while (!old_file.eof()) {
        Block block(table, old_file);
        if (block.getCount() != 0) {
            new_file << block.getBuffer();
        }
    }
    old_file.close();
    new_file.close();
    std::remove(getPathToTable(table_name).c_str());
    std::rename((getPathToTable(table_name) + salt).c_str(), getPathToTable(table_name).c_str());
}
