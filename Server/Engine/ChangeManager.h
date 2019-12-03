#ifndef OURSQL_CHANGEMANAGER_H
#define OURSQL_CHANGEMANAGER_H

#include <unordered_set>
#include <fstream>
#include <string>
#include <vector>
#include "Value.h"
#include "Table.h"
#include "Block.h"

enum ChangeType : char {
    inserted = 0,
    removed = 1
};

class ChangeManager {
   public:
    ChangeManager(const Table& table, int tr_id);
    void reset();
    bool nextInserted();
    std::vector<Value> fetch();
    void insert(const std::vector<Value>& values);
    void markRemoved();
    void remove(int position);
    int getRowSize() { return row_size_; }
    void setRowSize(int size) { row_size_ = size; }
    void markUpdate(bool is_update);

    bool next();
    int getChangeRowSize();
    ChangeType getChangeType();
    int getRemovedPosition();
    std::string getValues();
    void markProcessed();
    void moveToUnprocessed();

    static const int kNullEndPosition;

   private:
    int getUnprocessedPosition();
    void setUnprocessedPosition(int pos);

    static const int kUnprocessedPosition_;

    std::fstream file_;
    Table table_;
    int pos_ = 0;
    int end_pos_ = kNullEndPosition;
    int row_size_;
    bool was_file_finished_ = false;
};


#endif
