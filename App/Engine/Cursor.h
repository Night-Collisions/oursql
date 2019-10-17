#ifndef OURSQL_CURSOR_H
#define OURSQL_CURSOR_H

#include <vector>
#include <string>
#include <fstream>
#include "Column.h"
#include "Table.h"
#include "Block.h"
#include "Value.h"
#include "Engine.h"

class Cursor {
   public:
    Cursor(const std::string& table_name);
    ~Cursor();
    std::vector<Value> fetch();
    bool next();
    void insert(const std::vector<Value>& values);
    void update(const std::vector<Value>& values);
    void remove();

private:
    void saveBlock(Block& block, int n);
    std::fstream fstream_;
    Table table_;
    Block block_;
    int current_block_ = 0;
    bool was_block_changed_ = false;
};

#endif
