#ifndef OURSQL_TEST_H
#define OURSQL_TEST_H

#include <sstream>
#include <string>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../Server/Engine/Column.h"
#include "../Server/Engine/Table.h"

#define EXCEPTION2NUMB(expect) static_cast<unsigned int>(expect)

struct request_description {
    std::string request;
    long exception;
    std::string answer;
};

bool operator==(const Column& a, const Column& b);

bool operator==(const Table& a, const Table& b);

void clearDB();

void check_requests(const std::vector<request_description>& requests);

#endif
