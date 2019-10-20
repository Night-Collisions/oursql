#ifndef OURSQL_TEST_H
#define OURSQL_TEST_H

#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../Client/Client.h"

#define EXCEPTION2NUMB(expect) static_cast<unsigned int>(expect)

struct request_description {
    std::string request;
    long exception;
    std::string answer;
};

void clearDB();

void check_requests(const std::vector<request_description>& requests, Client&);

#endif
