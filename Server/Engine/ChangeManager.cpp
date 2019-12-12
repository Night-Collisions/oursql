#include "ChangeManager.h"

// File: table name: Engine::kTableNameLength bytes, unprocessed position: 4 bytes, rows
// Row: type : 1 byte,
//   removed: position: 4 byte
//   inserted: was removed: 1 byte, values: row size bytes

const int ChangeManager::kNullEndPosition = -1;
const int ChangeManager::kUnprocessedPosition_ = 128;

ChangeManager::ChangeManager(const Table& table, int tr_id) : table_(table) {
    std::string path = "DataBD/" + std::to_string(tr_id) + "/" + table_.getName();
    bool was_exist;
    {
        was_exist = static_cast<bool>(std::ifstream(path));
    }
    if (!was_exist) {
        file_.open(path, std::fstream::binary | std::fstream::out);
        const int name_length = 128;
        char buffer[name_length] = {0};
        std::memcpy(buffer, table_.getName().c_str(), table_.getName().size());
        file_.write(buffer, name_length);
        int unprocessed_position = 0;
        file_.write((char*) &unprocessed_position, sizeof(int));
        file_.flush();
        file_.close();
    }
    file_.open(path, std::fstream::binary | std::fstream::in | std::fstream::out);
    reset();
}

void ChangeManager::reset() {
    pos_ = 0;
    file_.seekp(0, std::fstream::end);
    was_file_finished_ = false;
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
    int p = file_.tellp();
    file_.seekp(0, std::fstream::end);
    char type = ChangeType::inserted;
    file_.write(&type, sizeof(char));
    char was_removed = 0;
    file_.write(&was_removed, sizeof(char));
    file_.write(Block::toRow(table_, values).rdbuf()->str().c_str(), getRowSize());
    file_.flush();
    file_.seekp(p);
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
    int p = file_.tellp();
    file_.seekp(0, std::fstream::end);
    char type = ChangeType::removed;
    file_.write(&type, sizeof(char));
    file_.write((char*) &position, sizeof(int));
    file_.flush();
    file_.seekp(p);
}

void ChangeManager::markUpdate(bool is_update) {
    int p = file_.tellp();
    file_.seekp(0, std::fstream::end);
    int end = file_.tellp();
    file_.seekp(p);
    end_pos_ = (is_update) ? (end) : (kNullEndPosition);
}

bool ChangeManager::next() {
    if (was_file_finished_) {
        return true;
    }
    int old_pos = pos_;
    pos_ = (pos_ == 0) ? (kUnprocessedPosition_ + sizeof(int)) : (pos_ + getChangeRowSize());
    if (pos_ == end_pos_) {
        pos_ = old_pos;
        return false;
    }
    file_.seekg(pos_);
    char type;
    file_.read(&type, sizeof(char));
    was_file_finished_ = file_.fail();
    if (was_file_finished_) {
        file_.clear();
    }
    file_.seekg(pos_);
    return !was_file_finished_;
}

bool ChangeManager::nextInserted() {
    while (next()) {
        if (getChangeType() == ChangeType::inserted) {
            return true;
        }
    }
    return false;
}

bool ChangeManager::nextRemoved() {
    while (next()) {
        if (getChangeType() == ChangeType::removed) {
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
