#include "Cursor.h"

Cursor::Cursor(const std::string& table_name) {
    fstream_.open(Engine::getPathToTable(table_name), std::ios::binary | std::ios::in | std::ios::out);
    std::unique_ptr<exc::Exception> e;
    table_ = Engine::show(table_name, e);
    block_ = Block(table_, fstream_);
}

Cursor::~Cursor() {
    if (was_block_changed_) {
        saveBlock(block_, current_block_);
    }
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
    while (!fstream_.eof()) {
        block_ = Block(table_, fstream_);
        ++current_block_;
        if (block_.next()) {
            return true;
        }
    }
    return false;
}

void Cursor::insert(const std::vector<Value>& values) {
    int g = fstream_.tellg();
    int p = fstream_.tellp();

    bool was_insert = false;
    fstream_.seekg(0);
    int num = 0;
    while (!fstream_.eof() && !was_insert) {
        Block block = Block(table_, fstream_);
        if (block.insert(values)) {
            was_insert = true;
            saveBlock(block, num);
        }
        ++num;
    }
    if (!was_insert) {
        Block block = Block(table_);
        block.insert(values);
        fstream_ << block.getBuffer();
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

void Cursor::saveBlock(Block block, int num) {
    int p = fstream_.tellp();
    fstream_.seekp(num * Block::kBlockSize);
    fstream_ << block.getBuffer();
    fstream_.seekp(p);
}

