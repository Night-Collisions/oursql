#include "Cursor.h"

Cursor::Cursor(int tr_id, const std::string& table_name) :
        tr_id_(tr_id),
        table_(Engine::show(table_name)),
        change_manager_(table_, tr_id) {
    block_.setTable(table_);
    file_position_ = 0;
    block_.load(file_position_);
    file_position_ += Block::kBlockSize;
    change_manager_.setRowSize(block_.getRowSize());
    loadRemovedRows();
}

void Cursor::loadRemovedRows() {
    while (change_manager_.nextRemoved()) {
        removed_rows_.insert(change_manager_.getRemovedPosition());
    }
    change_manager_.reset();
}

void Cursor::reset() {
    file_position_ = 0;
    block_.load(file_position_);
    file_position_ += Block::kBlockSize;
    change_manager_.reset();
    current_block_ = 0;
    was_file_finished_ = false;
}

std::vector<Value> Cursor::fetch() {
    return (was_file_finished_) ? (change_manager_.fetch()) : (block_.fetch());
}

bool Cursor::next() {
    while (!was_file_finished_ && block_.next(tr_id_)) {
        if (removed_rows_.find(current_block_ * Block::kBlockSize + block_.getPosition())
                == removed_rows_.end()) {
            return true;
        }
    }

    while (!was_file_finished_ && !block_.load(file_position_)) {
        file_position_ += Block::kBlockSize;
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
    std::fstream file(Engine::getPathToTable(table_.getName()), std::fstream::binary | std::fstream::in | std::fstream::out);
    file.seekg(-Block::kBlockSize, std::fstream::end);
    int last_block_start = file.tellg();
    file.close();
    block_.load(last_block_start);
    file.open(Engine::getPathToTable(table_.getName()), std::fstream::binary | std::fstream::in | std::fstream::out);
    file.seekp(-Block::kBlockSize, std::fstream::end);
    int last_block_id = file.tellp() / Block::kBlockSize;
    bool is_last_block = true;

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
                int p = file.tellp();
                file.seekp(change_manager_.getRemovedPosition() + Block::kTrEndIdPosition);
                int tr_end = Engine::getLastTransactionId();
                file.write((char*) &tr_end, sizeof(int));
                file.seekp(p);
            }
        } else {
            if (change_manager_.wasMarkedRemoved()) {
                continue;
            }
            if (!block_.insert(change_manager_.getValues(), Engine::getLastTransactionId() + 1)) {
                file.write(block_.getBuffer(), Block::kBlockSize);
                change_manager_.markProcessed();
                block_.reset();
                block_.insert(change_manager_.getValues(), Engine::getLastTransactionId() + 1);
                if (is_last_block) {
                    BuffersManager::updateBufferIfExists(table_.getName(), last_block_start, block_.getBuffer());
                }
                is_last_block = false;
            }
        }
    }

    if (block_.getCount() != 0) {
        if (is_last_block) {
            BuffersManager::updateBufferIfExists(table_.getName(), last_block_start, block_.getBuffer());
        }
        file.write(block_.getBuffer(), Block::kBlockSize);
        change_manager_.markProcessed();
    }
}
