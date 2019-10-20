#include "Block.h"


// delimiter: 1 byte;
// Block:
// rows count: 4 bytes;
// address of removed stack: 4 bytes;
// rows: state: 1 byte, fields
// field: mask: 1 byte: not null: 0 bit
//        integer: 4 bytes / real: 8 bytes / varchar(n): n bytes

enum State : char {
    empty = 0,
    exists = 1 << 0,
    removed = 1 << 1
};

Block::Block(const Table& table) {
    setTable(table);
    position_ = 8 - row_size_;
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

void Block::load(std::fstream& fstream) {
    fstream.read(buffer_, kBlockSize);
    position_ = 8 - row_size_;
}

int Block::getCount() const {
    int count;
    memcpy(&count, buffer_, sizeof(count));
    return count;
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
        value.is_null = (buffer_[pos] == 1);
        ++pos;
        switch (column.getType()) {
            case DataType::integer: {
                int i;
                memcpy(&i, &(buffer_[pos]), sizeof(i));
                value.data = std::to_string(i);
                pos += sizeof(i);
                break;
            }
            case DataType::real: {
                double d;
                memcpy(&d, &(buffer_[pos]), sizeof(d));
                value.data = std::to_string(d);
                pos += sizeof(d);
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
        buffer_[pos] = (values[i].is_null) ? (1) : (0);
        ++pos;
        switch (table_.getColumns()[i].getType()) {
            case DataType::integer: {
                int a = std::stoi(values[i].data);
                memcpy(&(buffer_[pos]), &a, sizeof(a));
                pos += sizeof(a);
                break;
            }
            case DataType::real: {
                double d = std::stod(values[i].data);
                memcpy(&(buffer_[pos]), &d, sizeof(d));
                pos += sizeof(d);
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
    buffer_[position_] = State::removed;
    memcpy(&(buffer_[position_ + 1]), &(buffer_[4]), 4);
    memcpy(&(buffer_[4]), &position_, 4);
}
