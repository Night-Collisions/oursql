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
    Block();
    ~Block() { delete[] buffer_; }
    Block(const Table& table);
    Block(const Table& table, std::fstream& fstream);
    void setTable(const Table& table);
    bool load(std::fstream& fstream);
    int getCount() const;
    void setCount(int count);
    int getPrevBlockId() const;
    int setPrevBlockId(int id);
    int getNextBlockId() const;
    int setNextBlockId(int id);
    bool next();
    std::vector<Value> fetch();
    bool insert(const std::vector<Value>& value);
    void update(const std::vector<Value>& values);
    void remove();
    const char* getBuffer() { return buffer_; }

    static const size_t kBlockSize;
    static const int kNullBlockId;
    static const int kRowsCountPosition;
    static const int kPrevBlockIdPosition;
    static const int kNextBlockIdPosition;

   private:
    void setValues(const std::vector<Value>& values, int pos);

    static const int kRemovedStackPosition_;
    static const int kRowsStartPosition_;

    char* buffer_ = new char[kBlockSize]{};
    Table table_;
    size_t row_size_;
    int position_;
};


#endif