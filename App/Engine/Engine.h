#ifndef OURSQL_APP_ENGINE_ENGINE_H_
#define OURSQL_APP_ENGINE_ENGINE_H_

#define RAPIDJSON_HAS_STDSTRING 1

#include <cstdio>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include "../Logic/Conditions/ConditionChecker.h"
#include "Table.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

class Engine {
   public:
    Engine() = delete;

    static void create(const Table& table, std::unique_ptr<exc::Exception>& e);

    static Table show(const std::string& name,
                      std::unique_ptr<exc::Exception>& e);

    static std::string showCreate(const std::string& name,
                                  std::unique_ptr<exc::Exception>& e);

    static void drop(const std::string& name,
                     std::unique_ptr<exc::Exception>& e);

    static bool exists(const std::string& name);

    static std::string getPathToTable(const std::string& name);

    static std::string getPathToTableMeta(const std::string& name);

    static void load(const std::string& name,
                     std::unique_ptr<exc::Exception>& e);

    static void commit(const std::string& name,
                       std::unique_ptr<exc::Exception>& e);

    static void free(const std::string& name,
                     std::unique_ptr<exc::Exception>& e);

    static rapidjson::Document select(const std::string& table,
                                      const std::set<std::string>& columns,
                                      const ConditionChecker& condition,
                                      std::unique_ptr<exc::Exception>& e);

    static void insert(const std::string& table, const std::unordered_map<std::string,
            std::string>& values, std::unique_ptr<exc::Exception>& e);

   private:
    static std::unordered_map<std::string, rapidjson::Document> loaded_tables_;
};

#endif
