#include "Block.h"

// Block:
// rows count: 4 bytes;
// address of removed stack: 4 bytes;
// rows: state: 1 byte, fields
// field: mask: 1 byte: not null: 0 bit
//        integer: value / real: value / varchar(n): 2 bytes: value: 2 bytes

enum State : char {
    empty = 0,
    exists = 1 << 0,
    removed = 1 << 1
};

Block::Block(Table table) : table_(std::move(table)) {
    row_size_ = 1 + table_.getColumns().size();
    for (const auto& column : table_.getColumns()) {
        switch (column.getType()) {
            case DataType::integer:
                row_size_ += 4;
                break;
            case DataType::real:
                row_size_ += 8;
                break;
            case DataType::varchar:
                row_size_ += column.getN();
                break;
        }
    }
    position_ = 8 - row_size_;
}

Block::Block(Table table, std::fstream& fstream) : Block(std::move(table)) {
    fstream.read(buffer_, kBlockSize);
}

int Block::getCount() const {
    return *((int*) buffer_[0]);
}

void Block::setCount(int count) {
    memcpy(buffer_, &count, sizeof(count));
}

bool Block::next() {
    position_ += row_size_;
    while (kBlockSize - position_ > row_size_) {
        if (buffer_[position_] == State::exists) {
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
        if ((buffer_[pos] & 1) != 0) {
            value.is_null = true;
        }
        ++pos;
        switch (column.getType()) {
            case DataType::integer: {
                value.data = std::to_string(*((int*) buffer_[pos]));
                pos += 4;
                break;
            }
            case DataType::real: {
                value.data = std::to_string(*((double *) buffer_[pos]));
                pos += 8;
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
    memcpy(&stack, &(buffer_[4]), 4);
    if (stack != 0) {
        pos = stack;
        memcpy(&(buffer_[4]), &(buffer_[stack + 1]), 4);
    }
    if (pos == -1) {
        pos = 8;
        while (true) {
            if (kBlockSize - pos < row_size_) {
                return false;
            }
            if (buffer_[pos] == State::empty) {
                break;
            }
            pos += row_size_;
        }
    }

    buffer_[pos] = State::exists;
    ++pos;
    for (int i = 0; i < row_size_ - 1; ++i) {
        buffer_[pos + i] = 0;
    }

    for (int i = 0; i < table_.getColumns().size(); ++i) {
        if (values[i].is_null) {
            buffer_[pos] = 1;
        }
        ++pos;
        switch (table_.getColumns()[i].getType()) {
            case DataType::integer: {
                int a = std::stoi(values[i].data);
                memcpy(&(buffer_[pos]), &a, sizeof(a));
                pos += 4;
                break;
            }
            case DataType::real: {
                double d = std::stod(values[i].data);
                memcpy(&(buffer_[pos]), &d, sizeof(d));
                pos += 8;
                break;
            }
            case DataType::varchar: {
                char s[table_.getColumns()[i].getN()];
                for (int j = 0; j < table_.getColumns()[i].getN(); ++j) {
                    s[j] = 0;
                }
                for (int j = 0; j < values[i].data.size(); ++j) {
                    s[j] = values[i].data[j];
                }
                memcpy(&(buffer_[pos]), s, table_.getColumns()[i].getN());
                pos += table_.getColumns()[i].getN();
                break;
            }
        }
    }

    return true;
}

void Block::update(const std::vector<Value>& values) {
    int pos = position_ + 1;
    for (int i = 0; i < row_size_ - 1; ++i) {
        buffer_[pos + i] = 0;
    }

    for (int i = 0; i < table_.getColumns().size(); ++i) {
        if (values[i].is_null) {
            buffer_[pos] = 1;
        }
        ++pos;
        switch (table_.getColumns()[i].getType()) {
            case DataType::integer: {
                int a = std::stoi(values[i].data);
                memcpy(&(buffer_[pos]), &a, sizeof(a));
                pos += 4;
                break;
            }
            case DataType::real: {
                double d = std::stod(values[i].data);
                memcpy(&(buffer_[pos]), &d, sizeof(d));
                pos += 8;
                break;
            }
            case DataType::varchar: {
                char s[table_.getColumns()[i].getN()];
                for (int j = 0; j < table_.getColumns()[i].getN(); ++j) {
                    s[j] = 0;
                }
                for (int j = 0; j < values[i].data.size(); ++j) {
                    s[j] = values[i].data[j];
                }
                memcpy(&(buffer_[pos]), s, table_.getColumns()[i].getN());
                pos += table_.getColumns()[i].getN();
                break;
            }
        }
    }
}

void Block::remove() {
    setCount(getCount() - 1);
    buffer_[position_] = State::removed;
    memcpy(&(buffer_[position_ + 1]), &(buffer_[4]), 4);
    memcpy(&(buffer_[4]), &position_, 4);
}
