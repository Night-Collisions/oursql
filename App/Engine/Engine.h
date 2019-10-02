#ifndef OURSQL_APP_ENGINE_ENGINE_H_
#define OURSQL_APP_ENGINE_ENGINE_H_

#include "Table.h"

bool create(const Table& table){};

Table show(const std::string& name);

std::string showCreate();

bool drop(const std::string& name);

bool exists(const std::string& name);

#endif
