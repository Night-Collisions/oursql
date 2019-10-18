#ifndef OURSQL_BLOCK_H
#define OURSQL_BLOCK_H

#include <algorithm>
#include <cstring>
#include <vector>
#include <array>
#include <fstream>
#include "Column.h"
#include "Value.h"
#include "Table.h"

class Block {
   public:
    Block() = default;
    ~Block() { delete[] buffer_; }
    Block(const Table& table);
    Block(const Table& table, std::fstream& fstream);
    void setTable(const Table& table);
    void load(std::fstream& fstream);
    int getCount() const;
    void setCount(int count);
    bool next();
    std::vector<Value> fetch();
    bool insert(const std::vector<Value>& value);
    void update(const std::vector<Value>& values);
    void remove();
    const char* getBuffer() { return buffer_; }

    static const size_t kBlockSize = 65536;

private:
    void setValues(const std::vector<Value>& values, int pos);

    char* buffer_ = new char[kBlockSize];
    Table table_;
    size_t row_size_;
    int position_;
};


#endif