#include "Cursor.h"

// Temporary file:
// Table name
// Blocks:
// number: 4 bytes, status: 4 bytes, block

Cursor::Cursor(const std::string& table_name) {
    file_.open(Engine::getPathToTable(table_name), std::ios::binary | std::ios::in | std::ios::out);
    std::unique_ptr<exc::Exception> e;
    table_ = Engine::show(table_name, e);
    char delimiter;
    file_ >> delimiter;
    block_.setTable(table_);
    block_.load(file_);
}

void Cursor::reset() {
    if (was_block_changed_) {
        was_block_changed_ = false;
        saveBlock(block_, current_block_);
    }
    file_.seekg(0, std::ios::beg);
    file_.seekp(0, std::ios::beg);
    char delimiter;
    file_ >> delimiter;
    block_.load(file_);
}

std::vector<Value> Cursor::fetch() {
    return block_.fetch();
}

bool Cursor::next() {
    if (block_.next()) {
        return true;
    }
    if (was_block_changed_) {
        was_block_changed_ = false;
        saveBlock(block_, current_block_);
    }
    char delimiter;
    while (file_ >> delimiter) {
        block_.load(file_);
        ++current_block_;
        if (block_.next()) {
            return true;
        }
    }

    if (file_.fail()) {
        file_.clear();
    }

    return false;
}

void Cursor::insert(const std::vector<Value>& values) {
    int g = file_.tellg();
    int p = file_.tellp();

    bool was_insert = false;
    file_.seekg(0, std::ios::beg);
    int num = 0;
    char delimiter;
    while (file_ >> delimiter && !was_insert) {
        Block block(table_, file_);
        if (block.insert(values)) {
            was_insert = true;
            saveBlock(block, num);
        }
        ++num;
    }
    if (!was_insert) {
        Block block(table_);
        block.insert(values);
        if (!tmp_file_.is_open()) {
            openTmpFile();
        }
        tmp_file_.seekg(0, std::ios::end);
        tmp_file_ << kNewBlockNumber_ << static_cast<int>(BlockStatus::new_block);
        tmp_file_.write(block.getBuffer(), Block::kBlockSize);
    }

    if (file_.fail()) {
        file_.clear();
    }
    file_.seekg(g);
    file_.seekp(p);
}

void Cursor::update(const std::vector<Value>& values) {
    was_block_changed_ = true;
    block_.update(values);
}

void Cursor::remove() {
    was_block_changed_ = true;
    block_.remove();
}

void Cursor::commit() {
    if (was_block_changed_) {
        was_block_changed_ = false;
        saveBlock(block_, current_block_);
    }

    Engine::setLastPerformingId(Engine::getLastCompletedId() + 1);
    tmp_file_.seekg(Engine::kTableNameLength);
    int number;
    int status;
    while (tmp_file_ >> number >> status) {
        auto blockStatus = static_cast<BlockStatus>(status);
        int p = file_.tellp();
        if (blockStatus == BlockStatus::updated_block) {
            file_.seekp(number * (1 + Block::kBlockSize) + 1);
        } else if (blockStatus == BlockStatus::new_block) {
            file_.seekp(0, std::ios::end);
            char delimiter = 0;
            file_ << delimiter;
        }
        char* buffer = new char[Block::kBlockSize];
        tmp_file_.read(buffer, Block::kBlockSize);
        file_.write(buffer, Block::kBlockSize);
        delete[] buffer;
        file_.seekp(p);
    }
    tmp_file_.clear();
    Engine::setLastCompletedId(Engine::getLastPerformingId());
}

void Cursor::saveBlock(Block& block, int num) {
    if (!tmp_file_.is_open()) {
        openTmpFile();
    }

    tmp_file_.seekg(Engine::kTableNameLength);
    bool is_new_tmp_block = true;
    int number;
    int status;
    while (tmp_file_ >> number >> status) {
        if (number == num) {
            is_new_tmp_block = false;
            tmp_file_.seekp(tmp_file_.tellg());
            tmp_file_.write(block.getBuffer(), Block::kBlockSize);
        }
    }
    tmp_file_.clear();

    if (is_new_tmp_block) {
        tmp_file_.seekp(0, std::ios::end);
        tmp_file_ << num << static_cast<int>(BlockStatus::updated_block);
        tmp_file_.write(block.getBuffer(), Block::kBlockSize);
    }
}

void Cursor::openTmpFile() {
    tmp_file_.open(
            Engine::kTmpTableFile,
            std::ios::binary | std::fstream::in | std::fstream::out | std::fstream::trunc
    );
    char table_name[Engine::kTableNameLength] = {0};
    std::memcpy(table_name, table_.getName().c_str(), table_.getName().size());
    tmp_file_.write(table_name, Engine::kTableNameLength);
}

