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

bool create(const Table& table);

Table show(const std::string& name);

std::string showCreate(const std::string& name);

bool drop(const std::string& name);

bool exists(const std::string& name);

std::string getPathToTable(const std::string& name);

std::string getPathToTableMeta(const std::string& name);

#endif
