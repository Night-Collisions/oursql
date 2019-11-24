#include "ChangeManager.h"

// File: table name: Engine::kTableNameLength bytes, row size: 4 bytes, unprocessed position: 4 bytes, rows
// Row: type : 1 byte,
//   removed: position: 4 byte
//   inserted: was removed: 1 byte, values: row size bytes

ChangeManager::ChangeManager(const Table& table, int tr_id) : table_(table) {
    std::string path = std::to_string(tr_id) + "/" + table_.getName();
    bool was_exist = static_cast<bool>(std::ifstream(path));
    file_.open(path,std::ios::binary | std::fstream::in | std::fstream::out);
    if (!was_exist) {
        const int name_length = 128;
        char buffer[name_length] = {0};
        std::memcpy(buffer, table_.getName().c_str(), table_.getName().size());
        file_.write(buffer, name_length);
        int row_size = 0;
        file_.write((char*) &row_size, sizeof(int));
        int unprocessed_position = kUnprocessedPosition_ + sizeof(int);
        file_.write((char*) &unprocessed_position, sizeof(int));
    }
    reset();
}

void ChangeManager::reset() {
    file_.seekg(kUnprocessedPosition_ + sizeof(int));
    file_.seekp(0, std::ios::end);
}

int ChangeManager::getRowSize() {
    int g = file_.tellg();
    file_.seekg(kRowSizePosition_);
    int size;
    file_.read((char*) &size, sizeof(int));
    file_.seekg(g);
    return size;
}

void ChangeManager::setRowSize(int size) {
    int p = file_.tellp();
    file_.seekp(kRowSizePosition_);
    file_.write((char*) &size, sizeof(int));
    file_.seekp(p);
}

int ChangeManager::getUnprocessedPosition() {
    int g = file_.tellg();
    file_.seekg(kUnprocessedPosition_);
    int pos;
    file_.read((char*) &pos, sizeof(int));
    file_.seekg(g);
    return pos;
}

void ChangeManager::setUnprocessedPosition(int pos) {
    int p = file_.tellp();
    file_.seekp(kUnprocessedPosition_);
    file_.write((char*) &pos, sizeof(int));
    file_.seekp(p);
}

std::vector<Value> ChangeManager::fetch() {
    int g = file_.tellg();
    char type;
    file_.read(&type, sizeof(char));
    char was_removed;
    file_.read(&was_removed, sizeof(char));
    int row_size = getRowSize();
    char buff[row_size];
    file_.read(buff, row_size);
    auto values = Block::toValues(table_, buff);
    file_.seekg(g);
    return values;
}

void ChangeManager::insert(const std::vector<Value>& values) {
    char type = ChangeType::inserted;
    file_.write(&type, sizeof(char));
    char was_removed = 0;
    file_.write(&was_removed, sizeof(char));
    file_.write(Block::toRow(table_, values).rdbuf()->str().c_str(), getRowSize());
    file_.flush();
}

void ChangeManager::markRemoved() {
    int p = file_.tellp();
    file_.seekp(sizeof(char) + file_.tellg());
    char was_removed = 1;
    file_.write(&was_removed, sizeof(char));
    file_.seekp(p);
    file_.flush();
}

void ChangeManager::remove(int position) {
    char type = ChangeType::removed;
    file_.write(&type, sizeof(char));
    file_.write((char*) &position, sizeof(int));
    file_.flush();
}

bool ChangeManager::next() {
    int g = getChangeRowSize() + file_.tellg();
    file_.seekg(g);
    char type;
    file_.read(&type, sizeof(char));
    bool was_file_finished = file_.fail();
    if (was_file_finished) {
        file_.clear();
    }
    file_.seekg(g);
    return !was_file_finished;
}

bool ChangeManager::nextInserted() {
    while (next()) {
        if (getChangeType() == ChangeType::inserted) {
            return true;
        }
    }
    return false;
}

ChangeType ChangeManager::getChangeType() {
    int g = file_.tellg();
    char type;
    file_.read(&type, sizeof(char));
    file_.seekg(g);
    return static_cast<ChangeType>(type);
}

int ChangeManager::getRemovedPosition() {
    int g = file_.tellg();
    file_.seekg(g + 1);
    int position;
    file_.read((char*) &position, sizeof(int));
    file_.seekg(g);
    return position;
}

std::string ChangeManager::getValues() {
    int g = file_.tellg();
    file_.seekg(g + 1 + 1);
    int row_size = getRowSize();
    char buff[row_size];
    file_.read(buff, row_size);
    file_.seekg(g);
    return std::string(buff, row_size);
}

int ChangeManager::getChangeRowSize() {
    return (getChangeType() == ChangeType::removed) ? (1 + 4) : (1 + 1 + getRowSize());
}

void ChangeManager::markProcessed() {
    setUnprocessedPosition(getChangeRowSize() + file_.tellg());
}

void ChangeManager::moveToUnprocessed() {
    file_.seekg(getUnprocessedPosition());
}
