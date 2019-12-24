#ifndef OURSQL_INDEXESMANAGER_H
#define OURSQL_INDEXESMANAGER_H

#include <unordered_map>
#include <vector>
#include <string>
#include "Index.h"

class IndexesManager {
   public:
    static bool exists(const std::string& tablename, int column_index);
    static Index* get(const std::string& tablename, int column_index);
    static void create(const Table& table, int column_index);
    static void drop(const std::string& tablename, int column_index);
    static void dropTable(const std::string& tablename);
    static void clear();
    static std::map<int, Index>* getTableIndexes(const std::string& tablename);

   private:
    static std::unordered_map<std::string, std::map<int, Index>> indexes;
};


#endif
