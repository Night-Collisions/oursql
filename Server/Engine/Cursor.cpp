#include "Cursor.h"

Cursor::Cursor(const std::string& table_name) {
    fstream_.open(Engine::getPathToTable(table_name), std::ios::binary | std::ios::in | std::ios::out);
    std::unique_ptr<exc::Exception> e;
    table_ = Engine::show(table_name, e);
    char delimiter;
    fstream_ >> delimiter;
    block_.setTable(table_);
    block_.load(fstream_);
}

Cursor::~Cursor() {
    if (was_block_changed_) {
        saveBlock(block_, current_block_);
    }
}

void Cursor::reset() {
    fstream_.seekg(0, std::ios::beg);
    fstream_.seekp(0, std::ios::beg);
    char delimiter;
    fstream_ >> delimiter;
    block_.load(fstream_);
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
    while (fstream_ >> delimiter) {
        block_.load(fstream_);
        ++current_block_;
        if (block_.next()) {
            return true;
        }
    }

    if (fstream_.fail()) {
        fstream_.clear();
    }

    return false;
}

void Cursor::insert(const std::vector<Value>& values) {
    int g = fstream_.tellg();
    int p = fstream_.tellp();

    bool was_insert = false;
    fstream_.seekg(0, std::ios::beg);
    int num = 0;
    char delimiter;
    while (fstream_ >> delimiter && !was_insert) {
        Block block(table_, fstream_);
        if (block.insert(values)) {
            was_insert = true;
            saveBlock(block, num);
        }
        ++num;
    }
    if (!was_insert) {
        char delimiter = 0;
        Block block(table_);
        block.insert(values);
        fstream_.seekg(0, std::ios::end);
        fstream_ << delimiter;
        fstream_.write(block.getBuffer(), Block::kBlockSize);
    }

    if (fstream_.fail()) {
        fstream_.clear();
    }
    fstream_.seekg(g);
    fstream_.seekp(p);
}

void Cursor::update(const std::vector<Value>& values) {
    was_block_changed_ = true;
    block_.update(values);
}

void Cursor::remove() {
    was_block_changed_ = true;
    block_.remove();
}

void Cursor::saveBlock(Block& block, int num) {
    int p = fstream_.tellp();
    fstream_.seekp(num * (1 + Block::kBlockSize) + 1);
    fstream_.write(block.getBuffer(), Block::kBlockSize);
    fstream_.seekp(p);
}

