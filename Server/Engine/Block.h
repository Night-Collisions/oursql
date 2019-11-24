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

class Block {
   public:
    static std::stringstream toRow(const Table& table, const std::vector<Value>& values);
    static std::vector<Value> toValues(const Table& table, char* buff);

    Block() = default;
    ~Block() { delete[] buffer_; }
    Block(const Table& table);
    Block(const Table& table, std::fstream& fstream);
    void setTable(const Table& table);
    bool load(std::fstream& fstream);
    int getCount() const;
    void setCount(int count);
    void setTrStartId(int id);
    void setTrEndId(int id);
    int getTrStartId();
    int getTrEndId();
    bool next(int id);
    std::vector<Value> fetch();
    bool insert(const std::vector<Value>& values, int id);
    bool insert(const std::string& values, int id);
    void remove(int id);
    const char* getBuffer() { return buffer_; }
    int getPosition() { return position_; }
    int getRowSize() { return row_size_; };

    static const size_t kBlockSize = 65536;
    static const int kRowsCountPosition = 0;

    static const int kTrStartIdPosition = 0;
    static const int kTrEndIdPosition = 4;

private:
    void setValues(const std::vector<Value>& values, int pos);

    static const int kRowsStartPosition_ = 4;

    char* buffer_ = new char[kBlockSize]{};
    Table table_;
    size_t row_size_;
    int position_;
};


#endif