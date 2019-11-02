#include "Cursor.h"

// Temporary file:
// Table name
// Blocks:
// number: 4 bytes, status: 4 bytes, block

Cursor::Cursor(const std::string& table_name) {
    file_.open(Engine::getPathToTable(table_name), std::ios::binary | std::ios::in | std::ios::out);
    std::unique_ptr<exc::Exception> e;
    table_ = Engine::show(table_name, e);
    setBlocksIds();
    block_.setTable(table_);
    if (current_block_ != Block::kNullBlockId) {
        file_.seekg(current_block_ * Block::kBlockSize);
    }
    block_.load(file_);
}

void Cursor::setBlocksIds() {
    int g = file_.tellg();

    int block_start = 0;
    file_.seekg(block_start);

    int prev;
    int next;
    int count;
    while (!file_.fail()) {
        file_.seekg(block_start + Block::kRowsCountPosition);
        file_.read((char*) &count, sizeof(int));
        file_.seekg(block_start + Block::kPrevBlockIdPosition);
        file_.read((char*) &prev, sizeof(int));
        file_.seekg(block_start + Block::kNextBlockIdPosition);
        file_.read((char*) &next, sizeof(int));

        if (next == Block::kNullBlockId) {
            if (count == 0) {
                last_empty_block_id_ = block_start / Block::kBlockSize;
            } else {
                last_non_empty_block_id_ = block_start / Block::kBlockSize;
            }
        }

        if (prev == Block::kNullBlockId && count != 0) {
            current_block_ = block_start / Block::kBlockSize;
        }

        block_start += Block::kBlockSize;
        file_.seekg(block_start);
        int fail;
        file_.read((char*) &fail, sizeof(int));
    }

    file_.clear();
    file_.seekg(g);
}

void Cursor::reset() {
    if (was_block_changed_) {
        saveBlock(block_, current_block_);
        was_block_changed_ = false;
    }
    setBlocksIds();
    if (current_block_ != Block::kNullBlockId) {
        file_.seekg(current_block_ * Block::kBlockSize);
    }
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
        saveBlock(block_, current_block_);
        was_block_changed_ = false;
    }

    if (current_block_ != Block::kNullBlockId) {
        current_block_ = getNextId(current_block_);
    }
    if (current_block_ == Block::kNullBlockId) {
        return false;
    }

    file_.seekg(current_block_ * Block::kBlockSize);
    block_.load(file_);
    block_.next();
    return true;
}

void Cursor::insert(const std::vector<Value>& values) {
    int g = file_.tellg();
    int p = file_.tellp();

    bool was_insert = false;
    file_.seekg(0, std::ios::beg);
    int num = 0;
    Block block(table_);
    while (!block.load(file_) && !was_insert) {
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
        int number = kNewBlockNumber_;
        tmp_file_.write((char*) &number, sizeof(int));
        int status = static_cast<int>(BlockStatus::new_block);
        tmp_file_.write((char*) &status, sizeof(int));
        tmp_file_.write(block.getBuffer(), Block::kBlockSize);
        tmp_file_.flush();
    }

    if (file_.fail()) {
        file_.clear();
    }
    file_.seekg(g);
    file_.seekp(p);
}

void Cursor::update(const std::vector<Value>& values) {
    block_.update(values);
    was_block_changed_ = true;
}

void Cursor::remove() {
    block_.remove();
    was_block_changed_ = true;
}

void Cursor::commit() {
    if (was_block_changed_) {
        saveBlock(block_, current_block_);
        was_block_changed_ = false;
    }

    Engine::setLastPerformingId(Engine::getLastCompletedId() + 1);
    tmp_file_.seekg(Engine::kTableNameLength);
    int id;
    int status;
    while (tmp_file_.read((char*) &id, sizeof(int))) {
        tmp_file_.read((char*) &status, sizeof(int));
        auto blockStatus = static_cast<BlockStatus>(status);
        int p = file_.tellp();
        if (blockStatus == BlockStatus::updated_block) {
            file_.seekp(id * Block::kBlockSize);
        } else if (blockStatus == BlockStatus::new_block) {
            file_.seekp(0, std::ios::end);
        }

        char* buffer = new char[Block::kBlockSize];
        tmp_file_.read(buffer, Block::kBlockSize);

        if (blockStatus == BlockStatus::updated_block) {
            int g = file_.tellg();
            file_.seekg(id * Block::kBlockSize + Block::kRowsCountPosition);
            int oldCount;
            file_.read((char *) &oldCount, sizeof(int));
            file_.seekg(g);
            int newCount;
            memcpy(&newCount, &(buffer[Block::kRowsCountPosition]), sizeof(int));

            if ((oldCount == 0) != (newCount == 0)) {
                if (getPrevId(id) != Block::kNullBlockId) {
                    setNextId(getPrevId(id), getNextId(id));
                }
                if (getNextId(id) != Block::kNullBlockId) {
                    setPrevId(getNextId(id), getPrevId(id));
                }

                if (newCount == 0) {
                    if (last_empty_block_id_ != Block::kNullBlockId) {
                        setNextId(last_empty_block_id_, id);
                    }
                    last_empty_block_id_ = id;
                } else {
                    if (last_non_empty_block_id_ != Block::kNullBlockId) {
                        setNextId(last_non_empty_block_id_, id);
                    }
                    last_non_empty_block_id_ = id;
                }
            }
        } else {
            int newCount;
            memcpy(&newCount, &(buffer[Block::kRowsCountPosition]), sizeof(int));

            if (newCount == 0) {
                if (last_empty_block_id_ != Block::kNullBlockId) {
                    setNextId(last_empty_block_id_, id);
                }
                last_empty_block_id_ = id;
            } else {
                if (last_non_empty_block_id_ != Block::kNullBlockId) {
                    setNextId(last_non_empty_block_id_, id);
                }
                last_non_empty_block_id_ = id;
            }
        }

        file_.write(buffer, Block::kBlockSize);
        delete[] buffer;
        file_.seekp(p);
    }
    tmp_file_.clear();
    Engine::setLastCompletedId(Engine::getLastPerformingId());
}

void Cursor::saveBlock(Block& block, int id) {
    if (!tmp_file_.is_open()) {
        openTmpFile();
    }

    tmp_file_.seekg(Engine::kTableNameLength);
    bool is_new_tmp_block = true;
    int existingId;
    int status;
    while (tmp_file_.read((char*) &existingId, sizeof(int))) {
        tmp_file_.read((char*) &status, sizeof(int));
        if (existingId == id) {
            is_new_tmp_block = false;
            tmp_file_.seekp(tmp_file_.tellg());
            tmp_file_.write(block.getBuffer(), Block::kBlockSize);
        }
    }
    tmp_file_.clear();

    if (is_new_tmp_block) {
        tmp_file_.seekp(0, std::ios::end);
        tmp_file_.write((char*) &id, sizeof(int));
        int status = static_cast<int>(BlockStatus::updated_block);
        tmp_file_.write((char*) &status, sizeof(int));
        tmp_file_.write(block.getBuffer(), Block::kBlockSize);
    }

    tmp_file_.flush();
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

int Cursor::getNextId(int id) {
    int g = file_.tellg();
    file_.seekg(id * Block::kBlockSize + Block::kNextBlockIdPosition);
    int nextId;
    file_.read((char*) &nextId, sizeof(int));
    file_.seekg(g);
    return nextId;
}

void Cursor::setNextId(int id, int nextId) {
    int p = file_.tellp();
    file_.seekp(id * Block::kBlockSize + Block::kNextBlockIdPosition);
    file_.write((char*) &nextId, sizeof(int));
    file_.seekp(p);
}

int Cursor::getPrevId(int id) {
    int g = file_.tellg();
    file_.seekg(id * Block::kBlockSize + Block::kPrevBlockIdPosition);
    int prevId;
    file_.read((char*) &prevId, sizeof(int));
    file_.seekg(g);
    return prevId;
}

void Cursor::setPrevId(int id, int prevId) {
    int p = file_.tellp();
    file_.seekp(id * Block::kBlockSize + Block::kPrevBlockIdPosition);
    file_.write((char*) &prevId, sizeof(int));
    file_.seekp(p);
}

