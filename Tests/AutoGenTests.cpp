#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "Test.h"
#include "parser.cpp"

TEST(AGT, TEST_1) {
    buffer_on = 1;
    parse_string("create table();");
    clearDB();
    EXPECT_EQ(buffer, "syntax error, unexpected LPAREN, expecting ID\nsyntax error, unexpected $end, expecting SEMI\n");
}

TEST(AGT, TEST_2) {
    buffer_on = 1;
    parse_string("create table a(b int);");
    clearDB();
    EXPECT_EQ(buffer, "");
}

TEST(AGT, TEST_3) {
    buffer_on = 1;
    parse_string("create table a(b int not null);");
    clearDB();
    EXPECT_EQ(buffer, "");
}

