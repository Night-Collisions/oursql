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

enum class BlockStatus : int {
    updated_block,
    new_block
};

class Cursor {
   public:
    Cursor(const std::string& table_name);
    void reset();
    std::vector<Value> fetch();
    bool next();
    void insert(const std::vector<Value>& values);
    void update(const std::vector<Value>& values);
    void remove();
    void commit();

private:
    void setBlocksIds();
    void saveBlock(Block& block, int id);
    void openTmpFile();
    int getNextId(int id);
    void setNextId(int id, int nextId);
    int getPrevId(int id);
    void setPrevId(int id, int prevId);

    static const int kNewBlockNumber_ = -1;
    std::fstream file_;
    std::fstream tmp_file_;
    Table table_;
    Block block_;
    int current_block_ = Block::kNullBlockId;
    bool was_block_changed_ = false;
    int last_empty_block_id_ = Block::kNullBlockId;
    int last_non_empty_block_id_ = Block::kNullBlockId;
};

#endif
