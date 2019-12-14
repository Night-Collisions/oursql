#include "Block.h"
#include "Engine.h"

// Block:
// rows count: 4 bytes;
// rows: tr start id: 4 bytes, tr end id: 4 bytes, fields
// field: null: 1 byte,
//        integer: 4 bytes / real: 8 bytes / varchar(n): n bytes

const int Block::kBlockSize = 65536;
const int Block::kRowsCountPosition = 0;

const int Block::kTrStartIdPosition = 0;
const int Block::kTrEndIdPosition = 4;

const int Block::kRowsStartPosition_ = 4;


enum ValueState : char {
    null = 0,
    not_null = 1
};

std::stringstream Block::toRow(const Table& table, const std::vector<Value>& values) {
    std::stringstream ss;

    for (int i = 0; i < table.getColumns().size(); ++i) {
        char isNull = (values[i].is_null) ? (ValueState::null) : (ValueState::not_null);
        ss.write(&isNull, sizeof(char));

        switch (table.getColumns()[i].getType()) {
            case DataType::integer: {
                int a = std::stoi(values[i].data);
                ss.write((char*) &a, sizeof(int));
                break;
            }
            case DataType::real: {
                double d = std::stod(values[i].data);
                ss.write((char*) &d, sizeof(double));
                break;
            }
            case DataType::varchar: {
                char s[table.getColumns()[i].getN()];
                for (int j = 0; j < values[i].data.size(); ++j) {
                    s[j] = values[i].data[j];
                }
                std::fill(s + values[i].data.size(), s + table.getColumns()[i].getN(), 0);
                ss.write(s, table.getColumns()[i].getN());
                break;
            }
            case DataType::datetime: {
                long long ll = std::stoll(values[i].data);
                ss.write((char*) &ll, sizeof(long long));
                break;
            }
        }
    }

    return ss;
}

std::vector<Value> Block::toValues(const Table& table, char* buff) {
    std::vector<Value> values;
    int pos = 0;

    for (const auto& column : table.getColumns()) {
        Value value;
        value.is_null = (buff[pos] == ValueState::null);
        ++pos;
        switch (column.getType()) {
            case DataType::integer: {
                int i;
                memcpy(&i, &(buff[pos]), sizeof(int));
                value.data = std::to_string(i);
                pos += sizeof(int);
                break;
            }
            case DataType::real: {
                double d;
                memcpy(&d, &(buff[pos]), sizeof(double));
                value.data = std::to_string(d);
                pos += sizeof(double);
                break;
            }
            case DataType::varchar: {
                char s[column.getN()];
                for (int i = 0; i < column.getN(); ++i) {
                    s[i] = buff[pos + i];
                }
                value.data = s;
                pos += column.getN();
                break;
            }
            case DataType::datetime: {
                long long ll;
                memcpy(&ll, &(buff[pos]), sizeof(long long));
                value.data = std::to_string(ll);
                pos += sizeof(long long);
                break;
            }
        }
        values.push_back(value);
    }

    return values;
}


Block::Block(const Table& table) {
    setTable(table);
    position_ = kRowsStartPosition_ - row_size_;
}

Block::Block(const Table& table, std::fstream& fstream) : Block(table) {
    load(fstream);
}

void Block::reset() {
    position_ = kRowsStartPosition_ - row_size_;
    for (int i = 0; i < kBlockSize; ++i) {
        buffer_[i] = 0;
    }
}

void Block::setTable(const Table& table) {
    table_ = table;
    row_size_ = 4 + 4 + table_.getColumns().size();
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
            case DataType::datetime:
                row_size_ ++ sizeof(long long);
                break;
        }
    }
}

bool Block::load(std::fstream& fstream) {
    fstream.read(buffer_, kBlockSize);
    position_ = kRowsStartPosition_ - row_size_;
    bool is_fail = fstream.fail();
    if (is_fail) {
        fstream.clear();
    }
    return is_fail;
}

int Block::getCount() const {
    int count;
    memcpy(&count, &(buffer_[kRowsCountPosition]), sizeof(int));
    return count;
}

void Block::setCount(int count) {
    memcpy(&(buffer_[kRowsCountPosition]), &count, sizeof(int));
}

void Block::setTrStartId(int id) {
    memcpy(&(buffer_[position_ + kTrStartIdPosition]), &id, sizeof(int));
}

void Block::setTrEndId(int id) {
    memcpy(&(buffer_[position_ + kTrEndIdPosition]), &id, sizeof(int));
}

int Block::getTrStartId() {
    int id;
    memcpy(&id, &(buffer_[position_ + kTrStartIdPosition]), sizeof(int));
    return id;
}

int Block::getTrEndId() {
    int id;
    memcpy(&id, &(buffer_[position_ + kTrEndIdPosition]), sizeof(int));
    return id;
}

bool Block::next(int id) {
    if (getCount() == 0) {
        return false;
    }

    position_ += row_size_;

    while (kBlockSize - position_ > row_size_) {
        if (getTrStartId() == Engine::kNullTransactionId) {
            return false;
        }
        if (getTrStartId() <= id
                && (getTrEndId() == Engine::kNullTransactionId || id <= getTrEndId())) {
            return true;
        }
        position_ += row_size_;
    }
    return false;
}

std::vector<Value> Block::fetch() {
    std::vector<Value> values;
    return toValues(table_, &(buffer_[position_ + 8]));
}

bool Block::insert(const std::vector<Value>& values, int id) {
    int pos = kRowsStartPosition_ + getCount() * row_size_;
    if (kBlockSize - pos < row_size_) {
        return false;
    }

    int tmp = position_;
    position_ = pos;
    setTrStartId(id);
    setTrEndId(Engine::kNullTransactionId);
    position_ = tmp;

    setValues(values, pos + 8);
    setCount(getCount() + 1);

    return true;
}

bool Block::insert(const std::string& values, int id) {
    int pos = kRowsStartPosition_ + getCount() * row_size_;
    if (kBlockSize - pos < row_size_) {
        return false;
    }

    int tmp = position_;
    position_ = pos;
    setTrStartId(id);
    setTrEndId(Engine::kNullTransactionId);
    position_ = tmp;
    int row_start = 8 + pos;
    std::fill(buffer_ + row_start, buffer_ + row_start + row_size_ - 8, 0);
    memcpy((char*) &(buffer_[row_start]), values.c_str(), row_size_ - 8);
    setCount(getCount() + 1);

    return true;
}

void Block::setValues(const std::vector<Value>& values, int pos) {
    std::fill(buffer_ + pos, buffer_ + pos + row_size_ - 8, 0);
    memcpy((char*) &(buffer_[pos]), (char*) toRow(table_, values).rdbuf(), row_size_ - 8);
}

void Block::remove(int id) {
    setTrEndId(id);
}
