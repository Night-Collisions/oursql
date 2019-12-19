#include "Engine.h"

// Metafile:
// is versioning: 1 byte;
// table name: 128 bytes (with '\0');
// columns count: 1 byte;
// columns: period: 1 byte, n: 4 bytes, type: 1 byte, constraints: 1 byte, column name: 128 bytes (with '\0').

namespace fs = boost::filesystem;

const int Engine::kTableNameLength = 128;
const int Engine::kNullTransactionId = 0;
const std::string Engine::kTransactionsIdsFile_ ("DataBD/transactions_ids");
const std::string Engine::kTransactionsEndTimesTable("transactions_end_times");
const size_t Engine::kColumnNameLength_ = 128;

std::mutex Engine::mutex_;
Engine::Initializer Engine::initializer_;

std::string Engine::getPathToTable(const std::string& table_name) {
    return "DataBD/" + table_name;
}

std::string Engine::getPathToTableMeta(const std::string& table_name) {
    return getPathToTable(table_name) + "_meta";
}

void Engine::initialize() {
    bool is_exist;
    {
        is_exist = static_cast<bool>(std::ifstream(kTransactionsIdsFile_));
    }
    if (!is_exist) {
        setIds(0, kNullTransactionId);
    }
    if (getPerformingTransactionId() != kNullTransactionId) {
        commitTransaction(getPerformingTransactionId());
    }
    if (!exists(kTransactionsEndTimesTable)) {
        createTransactionsEndTimesTable();
    }
}

void Engine::createTransactionsEndTimesTable() {
    std::unique_ptr<exc::Exception> ignore;
    std::vector<Column> columns;
    columns.emplace_back("transaction_id", DataType::integer, ignore);
    columns.emplace_back("end_time", DataType::datetime, ignore);
    create(Table(kTransactionsEndTimesTable, columns, ignore), ignore);
}

int Engine::generateNextTransactionId() {
    std::lock_guard<std::mutex> guard(mutex_);
    int nextId = getLastTransactionId() + 1;
    setLastTransactionId(nextId);
    return nextId;
}

void Engine::beginTransaction(int id) {
    fs::create_directory(fs::current_path() / "DataBD" / std::to_string(id));
}

void Engine::commitTransaction(int id) {
    std::lock_guard<std::mutex> guard(mutex_);
    setPerformingTransactionId(id);

    for (auto& p : fs::directory_iterator(fs::current_path() / "DataBD" / std::to_string(id))) {
        std::ifstream file("DataBD/" + std::to_string(id) + "/" + fs::path(p.path()).filename().string());
        char table_name_[kTableNameLength];
        file.read(table_name_, kTableNameLength);
        file.close();
        Cursor cursor(id, table_name_);
        cursor.commit();
    }

    insertIntoTransactionsEndTimesTable(id);
    setPerformingTransactionId(kNullTransactionId);
    endTransaction(id);
}

void Engine::insertIntoTransactionsEndTimesTable(int id){
    auto curr_time = boost::posix_time::second_clock::local_time();
    auto posix_time = boost::posix_time::to_time_t(curr_time);

    std::vector<Value> values;
    values.push_back({std::to_string(id)});
    values.push_back({std::to_string(posix_time)});
    Cursor cursor(0, kTransactionsEndTimesTable);
    cursor.insert(values);
    cursor.commit();
}


void Engine::endTransaction(int id) {
    fs::remove_all(fs::current_path() / "DataBD" / std::to_string(id));
}

int Engine::getLastTransactionId() {
    int lastTransactionId;
    int lastPerformingId;
    std::ifstream statusFile(kTransactionsIdsFile_);
    statusFile >> lastTransactionId >> lastPerformingId;
    return lastTransactionId;
}

int Engine::getPerformingTransactionId() {
    int lastTransactionId;
    int lastPerformingId;
    std::ifstream statusFile(kTransactionsIdsFile_);
    statusFile >> lastTransactionId >> lastPerformingId;
    return lastPerformingId;
}

void Engine::setLastTransactionId(int id) {
    setIds(id, getPerformingTransactionId());
}

void Engine::setPerformingTransactionId(int id) {
    setIds(getLastTransactionId(), id);
}

void Engine::setIds(int lastTransactionId, int lastPerformingTransactionId) {
    std::ofstream statusFile(kTransactionsIdsFile_);
    statusFile << lastTransactionId << " " << lastPerformingTransactionId;
}

void Engine::create(const Table& table, std::unique_ptr<exc::Exception>& e) {
    if (exists(table.getName())) {
        e.reset(new exc::cr_table::RepeatTableName(table.getName()));
        return;
    }

    try {
        std::ofstream metafile(getPathToTableMeta(table.getName()), std::ios::binary);
        unsigned char is_versioning = (table.isSystemVersioning()) ? (1) : (0);
        metafile.write((char*) &is_versioning, sizeof(unsigned char));
        char table_name[kTableNameLength] = {0};
        std::memcpy(table_name, table.getName().c_str(), table.getName().size());
        metafile.write(table_name, kTableNameLength);
        unsigned char columns_count = table.getColumns().size();
        metafile.write((char*) &columns_count, sizeof(unsigned char));

        for (const auto& column : table.getColumns()) {
            unsigned char period = static_cast<unsigned char>(column.getPeriod());
            metafile.write((char*) &period, sizeof(unsigned char));
            int n = column.getN();
            metafile.write((char*) &n, sizeof(int));
            unsigned char type = static_cast<unsigned char>(column.getType());
            metafile.write((char*) &type, sizeof(unsigned char));
            unsigned char constraints = column.getBitConstraint();
            metafile.write((char*) &constraints, sizeof(unsigned char));
            char column_name[kColumnNameLength_] = {0};
            std::memcpy(column_name, column.getName().c_str(), column.getName().size());
            metafile.write(column_name, kColumnNameLength_);
        }

        std::fstream file(getPathToTable(table.getName()), std::ios::binary | std::ios::out);
        Block block(table);
        file.write(block.getBuffer(), Block::kBlockSize);

    } catch (std::bad_alloc& bad_alloc) {
        e.reset(new exc::OutOfMemory());
        std::remove(getPathToTable(table.getName()).c_str());
        std::remove(getPathToTableMeta(table.getName()).c_str());
    }
}

Table Engine::show(const std::string& table_name) {
    std::ifstream metafile(getPathToTableMeta(table_name), std::ios::binary);
    Table table;

    unsigned char is_versioning;
    metafile.read((char*) &is_versioning, sizeof(unsigned char));
    table.setSystemVersioning(static_cast<bool>(is_versioning));
    char table_name_[kTableNameLength];
    metafile.read(table_name_, kTableNameLength);
    table.setName(table_name);
    unsigned char columns_count;
    metafile.read((char*) &columns_count, sizeof(unsigned char));

    for (int i = 0; i < columns_count; ++i) {
        unsigned char period;
        metafile.read((char*) &period, sizeof(unsigned char));
        int n;
        metafile.read((char*) &n, sizeof(n));
        unsigned char type;
        metafile.read((char*) &type, sizeof(unsigned char));
        unsigned char constraints;
        metafile.read((char*) &constraints, sizeof(unsigned char));
        char column_name[kTableNameLength];
        metafile.read(column_name, kColumnNameLength_);
        std::unique_ptr<exc::Exception> e;
        Column column(column_name, static_cast<DataType>(type), e, constraints);
        column.setN(n);
        column.setPeriod(static_cast<PeriodState>(period));
        table.addColumn(column, e);
    }

    return table;
}

std::string Engine::showCreate(const std::string& table_name, std::unique_ptr<exc::Exception>& e) {
    if (!exists(table_name)) {
        e.reset(new exc::acc::TableNonexistent(table_name));
        return std::string();
    }
    Table table = show(table_name);
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
