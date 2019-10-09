#ifndef OURSQL_TEST_H
#define OURSQL_TEST_H

#include <sstream>

#include "../App/Engine/Column.h"
#include "../App/Engine/Table.h"

bool operator==(const Column& a, const Column& b);

bool operator==(const Table& a, const Table& b);

void clearDB();

#endif
