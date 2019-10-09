#ifndef OURSQL_TEST_H
#define OURSQL_TEST_H

#include <sstream>

#include "../App/Engine/Column.h"
#include "../App/Engine/Table.h"

#define NO_EXCEPTION(exception) ASSERT_EQ(exception, 0);
#define CHECK_EXCEPTION(exception, excpect) \
    ASSERT_EQ(exception, static_cast<unsigned int>(excpect));

#define CHECK_REQUEST(request, exception, answer)         \
    std::stringstream in(request);                        \
    std::stringstream out;                                \
    CHECK_EXCEPTION(ourSQL::perform(in, out), exception); \
    EXPECT_EQ(out.str(), answer);

bool operator==(const Column& a, const Column& b);

bool operator==(const Table& a, const Table& b);

void clearDB();

#endif
