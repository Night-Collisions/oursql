#ifndef OURSQL_APP_ENGINE_ENGINE_H_
#define OURSQL_APP_ENGINE_ENGINE_H_

#include <boost/filesystem.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>
#include <mutex>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include "../Core/Exception.h"
#include "Table.h"
#include "Cursor.h"

class Engine {
   public:
    Engine() = delete;

    static void initialize();

    static void create(const Table& table, std::unique_ptr<exc::Exception>& e);

    static Table show(const std::string& table_name);

    static std::string showCreate(const std::string& table_name,
                                  std::unique_ptr<exc::Exception>& e);

    static void drop(const std::string& table_name,
                     std::unique_ptr<exc::Exception>& e);

    static bool exists(const std::string& table_name);

    static std::string getPathToTable(const std::string& table_name);

    static std::string getPathToTableMeta(const std::string& table_name);

    static int generateNextTransactionId();

    static void beginTransaction(int id);

    static void commitTransaction(int id);

    static void endTransaction(int id);

    static int getLastTransactionId();

    static int getPerformingTransactionId();

    static void setLastTransactionId(int id);

    static void setPerformingTransactionId(int id);

    static void setIds(int lastTransactionId, int lastPerformingTransactionId);

    static const int kTableNameLength;
    static const int kNullTransactionId;

    static const std::string kTransactionsEndTimesTable;

private:
    static void createTransactionsEndTimesTable();
    static void insertIntoTransactionsEndTimesTable(int id);

    static const std::string kTransactionsIdsFile_;
    static const size_t kColumnNameLength_;

    static std::mutex mutex_;

    static class Initializer {
     public:
        Initializer() { Engine::initialize(); }
    } initializer_;
};

#endif
