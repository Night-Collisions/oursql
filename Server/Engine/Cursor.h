#ifndef OURSQL_CURSOR_H
#define OURSQL_CURSOR_H

#include <unordered_set>
#include <vector>
#include <string>
#include <fstream>
#include "Column.h"
#include "Table.h"
#include "Block.h"
#include "Value.h"
#include "Engine.h"
#include "ChangeManager.h"
#include "IndexesManager.h"

class Cursor {
   public:
    static void createIndex(const Table& table, int column_index);

    Cursor(int tr_id, const std::string& table_name);
    void reset();
    std::vector<Value> fetch();
    bool next();
    bool untransactionedNext();
    void insert(const std::vector<Value>& values);
    void markUpdate(bool is_update) { change_manager_.markUpdate(is_update); }
    void update(const std::vector<Value>& values);
    void remove();
    void commit();
    void moveToUncommited();
    int getTrId() { return tr_id_; }
    bool wasRemoved(int position) {
        return removed_rows_.find(position) != removed_rows_.end();
    }
    int getPosition() { return current_block_ * Block::kBlockSize + block_.getPosition(); }
    std::pair<int, int> getTrBounds() { return std::make_pair(block_.getTrStartId(), block_.getTrEndId()); }
    bool existsInThisTransaction();
    void setPosition(int position);
    const Table& getTable() const { return table_; }

private:
    void loadRemovedRows();
    void insertIntoIndexes(const std::vector<Value>& values, int position);

    std::unordered_set<int> removed_rows_;
    int tr_id_;
    int file_position_ = 0;
    Table table_;
    ChangeManager change_manager_;
    Block block_;
    int current_block_ = 0;
    bool was_file_finished_ = false;
};

#endif
