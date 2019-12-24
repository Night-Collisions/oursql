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

class Cursor {
   public:
    Cursor(int tr_id, const std::string& table_name);
    void reset();
    std::vector<Value> fetch();
    bool next();
    void insert(const std::vector<Value>& values);
    void markUpdate(bool is_update) { change_manager_.markUpdate(is_update); }
    void update(const std::vector<Value>& values);
    void remove();
    void commit();
    int getTrId() { return tr_id_; }

private:
    void loadRemovedRows();

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
