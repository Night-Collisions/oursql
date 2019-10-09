#ifndef OURSQL_APP_ENGINE_ENGINE_H_
#define OURSQL_APP_ENGINE_ENGINE_H_

#define RAPIDJSON_HAS_STDSTRING 1

#include <fstream>
#include <sstream>
#include <cstdio>
#include "Table.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

class Engine {
public:
    Engine() = delete;

    static void create(const Table& table, std::unique_ptr<exc::Exception>& e);

    static Table show(const std::string& name, std::unique_ptr<exc::Exception>& e);

    static std::string showCreate(const std::string& name, std::unique_ptr<exc::Exception>& e);

    static void drop(const std::string& name, std::unique_ptr<exc::Exception>& e);

    static bool exists(const std::string& name);

    static std::string getPathToTable(const std::string& name);

    static std::string getPathToTableMeta(const std::string& name);

    static void load(const std::string& name, std::unique_ptr<exc::Exception>& e);

    static void commit(const std::string& name, std::unique_ptr<exc::Exception>& e);

    static void free(const std::string& name, std::unique_ptr<exc::Exception>& e);

private:
    std::unordered_map<std::string, rapidjson::Value> loaded_tables_;
};

#endif
