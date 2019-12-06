#include "Cursor.h"

Cursor::Cursor(int tr_id, const std::string& table_name) :
        tr_id_(tr_id),
        table_(Engine::show(table_name)),
        change_manager_(table_, tr_id) {
    file_.open(Engine::getPathToTable(table_name), std::fstream::binary | std::fstream::in | std::fstream::out);
    block_.setTable(table_);
    block_.load(file_);
    change_manager_.setRowSize(block_.getRowSize());
}

void Cursor::reset() {
    file_.seekg(0);
    change_manager_.reset();
    current_block_ = 0;
    was_file_finished_ = false;
    block_.load(file_);
}

std::vector<Value> Cursor::fetch() {
    return (was_file_finished_) ? (change_manager_.fetch()) : (block_.fetch());
}

bool Cursor::next() {
    while (block_.next(tr_id_)) {
        if (removed_rows_.find(current_block_ * Block::kBlockSize + block_.getPosition())
                == removed_rows_.end()) {
            return true;
        }
    }

    while (!block_.load(file_)) {
        ++current_block_;
        while (block_.next(tr_id_)) {
            if (removed_rows_.find(current_block_ * Block::kBlockSize + block_.getPosition())
                    == removed_rows_.end()) {
                return true;
            }
        }
    }

    was_file_finished_ = true;
    return change_manager_.nextInserted();
}

void Cursor::insert(const std::vector<Value>& values) {
    change_manager_.insert(values);
}

void Cursor::update(const std::vector<Value>& values) {
    remove();
    insert(values);
}

void Cursor::remove() {
    if (was_file_finished_) {
        change_manager_.markRemoved();
    } else {
        int position = current_block_ * Block::kBlockSize + block_.getPosition();
        change_manager_.remove(position);
        removed_rows_.insert(position);
    }
}

void Cursor::commit() {
    file_.seekg(-Block::kBlockSize, std::fstream::end);
    block_.load(file_);
    file_.seekp(-Block::kBlockSize, std::fstream::end);
    int last_block_id = file_.tellp() / Block::kBlockSize;
    bool is_last_block = true;;

    change_manager_.reset();
    change_manager_.moveToUnprocessed();

    while (change_manager_.next()) {
        if (change_manager_.getChangeType() == ChangeType::removed) {
            if (is_last_block && change_manager_.getRemovedPosition() / Block::kBlockSize == last_block_id) {
                int pos = block_.getPosition();
                block_.setPosition(change_manager_.getRemovedPosition() % Block::kBlockSize);
                block_.setTrEndId(Engine::getLastTransactionId());
                block_.setPosition(pos);
            } else {
                int p = file_.tellp();
                file_.seekp(change_manager_.getRemovedPosition() + Block::kTrEndIdPosition);
                int tr_end = Engine::getLastTransactionId();
                file_.write((char*) &tr_end, sizeof(int));
                file_.seekp(p);
            }
        } else {
            if (!block_.insert(change_manager_.getValues(), tr_id_)) {
                int p = file_.tellp();
                file_.seekp(0, std::fstream::end);
                file_.write(block_.getBuffer(), Block::kBlockSize);
                file_.seekp(p);
                change_manager_.markProcessed();
                block_ = Block(table_);
                block_.insert(change_manager_.getValues(), tr_id_);
                is_last_block = false;
            }
        }
    }

    if (block_.getCount() != 0) {
        file_.write(block_.getBuffer(), Block::kBlockSize);
        change_manager_.markProcessed();
    }
}
