#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../../App/Engine/Field.h"
#include "../../App/Engine/Table.h"
#include "parser.cpp"

extern Table getTable();

bool operator==(const Field& a, const Field& b) {
    return a.getName() == b.getName() && a.getType() == b.getType() &&
           a.getConstraint() == b.getConstraint();
}

bool operator==(const Table& a, const Table& b) {
    auto a_fields = a.getFields();
    auto b_fields = b.getFields();
    if (a.getName() != b.getName() || a_fields.size() != b_fields.size()) {
        return false;
    }
    for (const auto& i : a_fields) {
        bool eq = false;
        for (const auto& j : b_fields) {
            if (i == j) {
                eq = true;
                break;
            }
        }
        if (!eq) {
            return false;
        }
    }
    return true;
}

TEST(Parser_CreateTable, SimpleTest) {
    ASSERT_FALSE(parse_string("create table MyTable(Name text);\n"));
    auto table = getTable();
    Table expect_table("MyTable", {{"Name", DataType::text}});
    EXPECT_EQ(table, expect_table);
}

TEST(Parser_CreateTable, UPCASE) {
    parse_string("CREATE TABLE MyTablE(DatA REAL);\n");
    ASSERT_FALSE(parse_string("CREATE TABLE MyTablE(DatA REAL);\n"));
    auto table = getTable();
    Table expect_table("MyTablE", {{"DatA", DataType::real}});
    EXPECT_EQ(table, expect_table);
}

TEST(Parser_CreateTable, AnySpase) {
    ASSERT_FALSE(parse_string(
        "create    \ntable a   (  \n  f  int   ,  h   real  , E   text )   ;   "
        "\n"));
    auto table = getTable();
    Table expect_table("a", {{"f", DataType::integer},
                             {"h", DataType::real},
                             {"E", DataType::text}});
    EXPECT_EQ(table, expect_table);
}

TEST(Parser_CreateTable, EqDataType) {
    ASSERT_FALSE(parse_string("create table a(b int, h int);\n"));
    auto table = getTable();
    Table expect_table("a",
                       {{"b", DataType::integer}, {"h", DataType::integer}});
    EXPECT_EQ(table, expect_table);
}

TEST(Parser_CreateTable, WrongStyleComand) {
    ASSERT_TRUE(parse_string("Create tAbLE MyTable(i Int);\n"));
}

TEST(Parser_CreateTable, WrongDataType1) {
    ASSERT_TRUE(parse_string("create table MyTable(create table MyTable);\n"));
}

TEST(Parser_CreateTable, WrongDataType2) {
    ASSERT_TRUE(parse_string("create table MyTable(Name rael);\n"));
}

TEST(Parser_CreateTable, WrongParam1) {
    ASSERT_TRUE(parse_string("create table MyTable();\n"));
}

TEST(Parser_CreateTable, WrongParam2) {
    ASSERT_TRUE(parse_string("create table MyTable(int);\n"));
}

TEST(Parser_CreateTable, WrongParam3) {
    ASSERT_TRUE(parse_string("create table MyTable(a);\n"));
}

TEST(Parser_CreateTable, WrongParam4) {
    ASSERT_TRUE(parse_string("create table MyTable(a int;);\n"));
}

TEST(Parser_CreateTable, WithoutSemicolon) {
    ASSERT_TRUE(parse_string("create table MyTable(a int)\n"));
}

TEST(Parser_CreateTable, TwoCreate) {
    ASSERT_TRUE(parse_string(
        "create table MyTable1(a int); \n create table MyTable2(a int);\n"));
}

TEST(Parser_CreateTable, WithOutPoints) {
    ASSERT_TRUE(parse_string("create table MyTable(Name text)\n"));
}

TEST(Parser_CreateTable, IntNameOfField) {
    ASSERT_TRUE(parse_string("create table a(int int, h int);\n"));
}

TEST(Parser_CreateTable, WithWalue) {
    ASSERT_TRUE(
        parse_string("create table MyTable(Name text(5), Status int(2));\n"));
}

TEST(Parser_ShowTable, SimpleTest) {
    ASSERT_FALSE(parse_string("show table a;\n"));
}

TEST(Parser_ShowTable, UPCASE) {
    ASSERT_FALSE(parse_string("SHOW TABLE A;\n"));
}

TEST(Parser_ShowTable, AnySpase) {
    ASSERT_FALSE(parse_string("  \n  SHOW     TABLE  \n  A  ; \n"));
}

TEST(Parser_ShowTable, WrongShow1) {
    ASSERT_TRUE(parse_string("SHOW TABLE A(f int);\n"));
}

TEST(Parser_ShowTable, WrongShow2) {
    ASSERT_TRUE(parse_string("SHOW TABLE A();\n"));
}
