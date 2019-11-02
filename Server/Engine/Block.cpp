#include "Block.h"

// delimiter: 1 byte;
// Block:
// rows count: 4 bytes;
// address of removed stack: 4 bytes;
// rows: state: 1 byte, fields
// field: mask: 1 byte: not null: 0 bit
//        integer: 4 bytes / real: 8 bytes / varchar(n): n bytes

const size_t Block::kBlockSize = 65536;
const int Block::kNullBlockId = -1;

const int Block::kRowsCountPosition = 0;
const int Block::kRemovedStackPosition_ = 4;
const int Block::kPrevBlockIdPosition = 8;
const int Block::kNextBlockIdPosition = 12;
const int Block::kRowsStartPosition_ = 16;

enum RowState : char {
    empty = 0,
    exists = 1 << 0,
    removed = 1 << 1
};

enum ValueState : char {
    null = 0,
    not_null = 1
};

Block::Block() {
    setPrevBlockId(kNullBlockId);
    setNextBlockId(kNullBlockId);
}

Block::Block(const Table& table) : Block() {
    setTable(table);
    position_ = kRowsStartPosition_ - row_size_;
}

Block::Block(const Table& table, std::fstream& fstream) : Block(table) {
    load(fstream);
}

void Block::setTable(const Table& table) {
    table_ = table;
    row_size_ = 1 + table_.getColumns().size();
    for (const auto& column : table_.getColumns()) {
        switch (column.getType()) {
            case DataType::integer:
                row_size_ += sizeof(int);
                break;
            case DataType::real:
                row_size_ += sizeof(double);
                break;
            case DataType::varchar:
                row_size_ += column.getN();
                break;
        }
    }
}

bool Block::load(std::fstream& fstream) {
    fstream.read(buffer_, kBlockSize);
    position_ = kRowsStartPosition_ - row_size_;
    return fstream.fail();
}

int Block::getCount() const {
    int count;
    memcpy(&count, &(buffer_[kRowsCountPosition]), sizeof(int));
    return count;
}

void Block::setCount(int count) {
    memcpy(&(buffer_[kRowsCountPosition]), &count, sizeof(int));
}

int Block::getPrevBlockId() const {
    int id;
    memcpy(&id, &(buffer_[kPrevBlockIdPosition]), sizeof(int));
    return id;
}

int Block::setPrevBlockId(int id) {
    memcpy(&(buffer_[kPrevBlockIdPosition]), &id, sizeof(int));
}

int Block::getNextBlockId() const {
    int id;
    memcpy(&id, &(buffer_[kNextBlockIdPosition]), sizeof(int));
    return id;
}

int Block::setNextBlockId(int id) {
    memcpy(&(buffer_[kNextBlockIdPosition]), &id, sizeof(int));
}

bool Block::next() {
    if (getCount() == 0) {
        return false;
    }

    position_ += row_size_;

    while (kBlockSize - position_ > row_size_) {
        if (buffer_[position_] == RowState::exists) {
            return true;
        }
        position_ += row_size_;
    }
    return false;
}

std::vector<Value> Block::fetch() {
    std::vector<Value> values;
    int pos = position_ + 1;

    for (const auto& column : table_.getColumns()) {
        Value value;
        value.is_null = (buffer_[pos] == ValueState::null);
        ++pos;
        switch (column.getType()) {
            case DataType::integer: {
                int i;
                memcpy(&i, &(buffer_[pos]), sizeof(int));
                value.data = std::to_string(i);
                pos += sizeof(int);
                break;
            }
            case DataType::real: {
                double d;
                memcpy(&d, &(buffer_[pos]), sizeof(double));
                value.data = std::to_string(d);
                pos += sizeof(double);
                break;
            }
            case DataType::varchar:
                char s[column.getN()];
                for (int i = 0; i < column.getN(); ++i) {
                    s[i] = buffer_[pos + i];
                }
                value.data = s;
                pos += column.getN();
                break;
        }
        values.push_back(value);
    }

    return values;
}

bool Block::insert(const std::vector<Value>& values) {
    int pos = -1;
    int stack;
    memcpy(&stack, &(buffer_[kRemovedStackPosition_]), sizeof(int));
    if (stack != 0) {
        pos = stack;
        memcpy(&(buffer_[kRemovedStackPosition_]), &(buffer_[stack + 1]), sizeof(int));
    }
    if (pos == -1) {
        pos = kRowsStartPosition_;
        while (true) {
            if (kBlockSize - pos < row_size_) {
                return false;
            }
            if (buffer_[pos] == RowState::empty) {
                break;
            }
            pos += row_size_;
        }
    }

    buffer_[pos] = RowState::exists;
    setValues(values, pos + 1);
    setCount(getCount() + 1);

    return true;
}

void Block::update(const std::vector<Value>& values) {
    setValues(values, position_ + 1);
}

void Block::setValues(const std::vector<Value>& values, int pos) {
    std::fill(buffer_ + pos, buffer_ + pos + row_size_ - 1, 0);

    for (int i = 0; i < table_.getColumns().size(); ++i) {
        buffer_[pos] = (values[i].is_null) ? (ValueState::null) : (ValueState::not_null);
        ++pos;
        switch (table_.getColumns()[i].getType()) {
            case DataType::integer: {
                int a = std::stoi(values[i].data);
                memcpy(&(buffer_[pos]), &a, sizeof(int));
                pos += sizeof(int);
                break;
            }
            case DataType::real: {
                double d = std::stod(values[i].data);
                memcpy(&(buffer_[pos]), &d, sizeof(double));
                pos += sizeof(double);
                break;
            }
            case DataType::varchar: {
                char s[table_.getColumns()[i].getN()];
                for (int j = 0; j < values[i].data.size(); ++j) {
                    s[j] = values[i].data[j];
                }
                std::fill(s + values[i].data.size(), s + table_.getColumns()[i].getN(), 0);
                memcpy(&(buffer_[pos]), s, table_.getColumns()[i].getN());
                pos += table_.getColumns()[i].getN();
                break;
            }
        }
    }
}

void Block::remove() {
    setCount(getCount() - 1);
    buffer_[position_] = RowState::removed;
    memcpy(&(buffer_[position_ + 1]), &(buffer_[kRemovedStackPosition_]), sizeof(int));
    memcpy(&(buffer_[kRemovedStackPosition_]), &position_, sizeof(int));
}
