#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "parser.cpp"

TEST(AGT, TEST_1) {
    parse_string("show tables;");
    //  TODO: фигня для провекри ответа
}

TEST(AGT, TEST_2) {
    parse_string("drop table a;");
    //  TODO: фигня для провекри ответа
}

TEST(AGT, TEST_3) {
    parse_string("create table b ( a real);");
    //  TODO: фигня для провекри ответа
}

TEST(AGT, TEST_4) {
    parse_string("create");
    //  TODO: фигня для провекри ответа
}

