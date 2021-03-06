#ifndef OURSQL_BLOCK_H
#define OURSQL_BLOCK_H

#include <algorithm>
#include <cstring>
#include <vector>
#include <array>
#include <fstream>
#include <sstream>
#include "Column.h"
#include "Value.h"
#include "Table.h"
#include "BuffersManager.h"

class Block {
   public:
    static std::stringstream toRow(const Table& table, const std::vector<Value>& values);
    static std::vector<Value> toValues(const Table& table, const char* buff);

    Block() = default;
    ~Block() { delete[] buffer_; }
    Block(const Table& table);
    void reset();
    void setTable(const Table& table);
//    bool load(std::fstream& fstream);
    bool load(int start);
    int getCount() const;
    void setCount(int count);
    void setTrStartId(int id);
    void setTrEndId(int id);
    int getTrStartId();
    int getTrEndId();
    bool next(int id);
    bool untransactionedNext();
    std::vector<Value> fetch();
    bool insert(const std::vector<Value>& values, int id);
    bool insert(const std::string& values, int id);
    void remove(int id);
    const char* getBuffer() { return buffer_; }
    void setPosition(int position) { position_ = position; }
    int getPosition() { return position_; }
    int getRowSize() { return row_size_ - 4 - 4; };
    int getInsertionPosition() { return kRowsStartPosition_ + getCount() * row_size_; }

    static const int kBlockSize;
    static const int kRowsCountPosition;

    static const int kTrStartIdPosition;
    static const int kTrEndIdPosition;

private:
    void setValues(const std::vector<Value>& values, int pos);

    static const int kRowsStartPosition_;

    char* buffer_ = new char[kBlockSize]{};
    Table table_;
    size_t row_size_;
    int loadedBlockStart_ = -1;
    int position_;
};


#endif