#include "Cursor.h"

void Cursor::createIndex(const Table& table, int column_index) {
    IndexesManager::create(table, column_index);
    Index* index = IndexesManager::get(table.getName(), column_index);
    Cursor cursor(0, table.getName());
    while (cursor.untransactionedNext()) {
        index->getPositions().insert(
                std::make_pair(cursor.fetch()[index->getIndex()].data, cursor.getPosition())
        );
    }
}

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

bool Cursor::untransactionedNext() {
    while (!was_file_finished_ && block_.untransactionedNext()) {
        return true;
    }

    if (!was_file_finished_ && !block_.load(file_position_)) {
        file_position_ += Block::kBlockSize;
        ++current_block_;
        block_.next(tr_id_);
        return true;
    }
    was_file_finished_ = true;
    return false;
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

void Cursor::moveToUncommited() {
    was_file_finished_ = true;
    change_manager_.reset();
}

bool Cursor::existsInThisTransaction() {
    auto bounds = getTrBounds();
    int pos = current_block_ * Block::kBlockSize + block_.getPosition();
    return bounds.first <= getTrId() &&
            (bounds.second == Engine::kNullTransactionId || getTrId() <= bounds.second) &&
            removed_rows_.find(pos) == removed_rows_.end();

}

void Cursor::setPosition(int position) {
    current_block_ = position / Block::kBlockSize;
    file_position_ = current_block_ * Block::kBlockSize;
    block_.load(file_position_);
    block_.setPosition(position % Block::kBlockSize);
    file_position_ += Block::kBlockSize;
    change_manager_.reset();
    was_file_finished_ = false;
}

void Cursor::commit() {
    std::fstream file(Engine::getPathToTable(
            table_.getName()), std::fstream::binary | std::fstream::in | std::fstream::out
    );
    file.seekg(-Block::kBlockSize, std::fstream::end);
    int last_block_start = file.tellg();
    file.close();
    block_.load(last_block_start);
    file.open(Engine::getPathToTable(
            table_.getName()), std::fstream::binary | std::fstream::in | std::fstream::out
    );
    file.seekp(-Block::kBlockSize, std::fstream::end);
    int last_block_id = file.tellp() / Block::kBlockSize;
    int block_start_position = file.tellp();
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
            auto values = Block::toValues(table_, change_manager_.getValues().c_str());
            insertIntoIndexes(values, block_start_position + block_.getInsertionPosition());
            if (!block_.insert(change_manager_.getValues(), Engine::getLastTransactionId() + 1)) {
                file.write(block_.getBuffer(), Block::kBlockSize);
                change_manager_.markProcessed();
                block_.reset();
                block_.insert(change_manager_.getValues(), Engine::getLastTransactionId() + 1);
                if (is_last_block) {
                    BuffersManager::updateBufferIfExists(table_.getName(), last_block_start, block_.getBuffer());
                }
                is_last_block = false;
                block_start_position += Block::kBlockSize;
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

void Cursor::insertIntoIndexes(const std::vector<Value>& values, int position) {
    auto indexes = IndexesManager::getTableIndexes(table_.getName());
    if (indexes == nullptr) {
        return;
    }
    for (auto it : *indexes) {
        it.second.getPositions().insert(std::make_pair(values[it.second.getIndex()].data, position));
    }
}