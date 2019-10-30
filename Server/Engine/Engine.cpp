#include "Engine.h"
#include "../Core/Exception.h"

// Metafile:
// reserved: 1 byte;
// table name: 128 bytes (with '\0');
// columns count: 1 byte;
// columns: reserved: 1 byte, n: 4 bytes, type: 1 byte, constraints: 1 byte, column name: 128 bytes (with '\0').

Engine::Initializer Engine::initializer_;

std::string Engine::getPathToTable(const std::string& table_name) {
    return "DataBD/" + table_name;
}

std::string Engine::getPathToTableMeta(const std::string& table_name) {
    return getPathToTable(table_name) + "_meta";
}

void Engine::initialize() {
    if (!static_cast<bool>(std::ifstream(kStatusFile_))) {
        setIds(0, 0);
    }
    if (getLastCompletedId() != getLastPerformingId()) {
        std::ifstream tmp_file(kTmpTableFile);
        char table_name_[kTableNameLength];
        tmp_file.read(table_name_, kTableNameLength);
        tmp_file.close();
        Cursor cursor(table_name_);
        cursor.commit();
    }
}

int Engine::getLastCompletedId() {
    int id;
    std::ifstream statusFile(kStatusFile_);
    statusFile >> id;
    return id;
}

int Engine::getLastPerformingId() {
    int id;
    std::ifstream statusFile(kStatusFile_);
    for (int i = 0; i < 2; ++i) {
        statusFile >> id;
    }
    return id;
}

void Engine::setLastCompletedId(int id) {
    setIds(id, getLastPerformingId());
}

void Engine::setLastPerformingId(int id) {
    setIds(getLastCompletedId(), id);
}

void Engine::setIds(int lastCompletedId, int lastPerformingId) {
    std::ofstream statusFile(kStatusFile_);
    statusFile << lastCompletedId << lastPerformingId;
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
        char table_name[kTableNameLength] = {0};
        std::memcpy(table_name, table.getName().c_str(), table.getName().size());
        metafile.write(table_name, kTableNameLength);
        unsigned char columns_count = table.getColumns().size();
        metafile << columns_count;

        for (const auto& column : table.getColumns()) {
            unsigned char reserved = 0;
            metafile << reserved;
            int n = column.getN();
            metafile.write((char*) &n, sizeof(n));
            unsigned char type = static_cast<unsigned char>(column.getType());
            metafile << type;
            unsigned char constraints = column.getBitConstraint();
            metafile << constraints;
            char column_name[kColumnNameLength_] = {0};
            std::memcpy(column_name, column.getName().c_str(), column.getName().size());
            metafile.write(column_name, kColumnNameLength_);
        }

        std::fstream file(getPathToTable(table.getName()), std::ios::binary | std::ios::out);
        char delimiter = 0;
        file << delimiter;
        Block block(table);
        file.write(block.getBuffer(), Block::kBlockSize);

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
    char table_name_[kTableNameLength];
    metafile.read(table_name_, kTableNameLength);
    table.setName(table_name);
    unsigned char columns_count;
    metafile >> columns_count;

    for (int i = 0; i < columns_count; ++i) {
        unsigned char reserved;
        metafile >> reserved;
        int n;
        metafile.read((char*) &n, sizeof(n));
        unsigned char type;
        metafile >> type;
        unsigned char constraints;
        metafile >> constraints;
        char column_name[kTableNameLength];
        metafile.read(column_name, kColumnNameLength_);
        std::unique_ptr<exc::Exception> e;
        Column column(column_name, static_cast<DataType>(type), e, constraints);
        column.setN(n);
        table.addColumn(column, e);
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
        if (column.getType() == DataType::varchar) {
            query.append("(" + std::to_string(column.getN()) + ")");
        }

        unsigned char mask = 1;
        while (mask != 0) {
            unsigned char constraint = mask & column.getBitConstraint();
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
