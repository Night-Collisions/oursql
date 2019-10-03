#ifndef OURSQL_TEST_H
#define OURSQL_TEST_H

#include "../App/Engine/Field.h"
#include "../App/Engine/Table.h"

bool operator==(const Field& a, const Field& b);

bool operator==(const Table& a, const Table& b);

#endif
