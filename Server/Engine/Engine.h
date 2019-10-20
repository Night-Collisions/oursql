#ifndef OURSQL_APP_ENGINE_ENGINE_H_
#define OURSQL_APP_ENGINE_ENGINE_H_

#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include "Table.h"
#include "Cursor.h"

class Engine {
   public:
    Engine() = delete;

    static void create(const Table& table, std::unique_ptr<exc::Exception>& e);

    static Table show(const std::string& table_name,
                      std::unique_ptr<exc::Exception>& e);

    static std::string showCreate(const std::string& table_name,
                                  std::unique_ptr<exc::Exception>& e);

    static void drop(const std::string& table_name,
                     std::unique_ptr<exc::Exception>& e);

    static bool exists(const std::string& table_name);

    static std::string getPathToTable(const std::string& table_name);

    static std::string getPathToTableMeta(const std::string& table_name);

    static void freeMemory(const std::string& table_name);

   private:
    static const size_t kTableNameLength_ = 128;
    static const size_t kColumnNameLength_ = 128;
};

#endif
