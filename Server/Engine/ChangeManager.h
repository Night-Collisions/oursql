#ifndef OURSQL_CHANGEMANAGER_H
#define OURSQL_CHANGEMANAGER_H

#include <unordered_set>
#include <fstream>
#include <string>
#include <vector>
#include "Engine.h"
#include "Value.h"

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
    int getRowSize();
    void setRowSize(int size);

    bool next();
    int getChangeRowSize();
    ChangeType getChangeType();
    int getRemovedPosition();
    std::string getValues();
    void markProcessed();
    void moveToUnprocessed();

   private:
    int getUnprocessedPosition();
    void setUnprocessedPosition(int pos);

    static const int kRowSizePosition_ = Engine::kTableNameLength;
    static const int kUnprocessedPosition_ = kRowSizePosition_ + sizeof(int);

    std::fstream file_;
    Table table_;
};


#endif
