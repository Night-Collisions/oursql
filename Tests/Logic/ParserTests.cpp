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

void setParserRequest(const std::string& s) {
    FILE* f;
    f = tmpfile();
    fwrite(s.c_str(), 1, s.size(), f);
    rewind(f);
    yyin = f;
}

TEST(Parser_CreateTable, SimpleTest) {
    setParserRequest("create table MyTable(Name text);\n");
    ASSERT_FALSE(yyparse());
    auto table = getTable();
    Table expect_table("MyTable", {{"Name", DataType::text}});
    EXPECT_EQ(table, expect_table);
}

TEST(Parser_CreateTable, UPCASE) {
    setParserRequest("CREATE TABLE MyTablE(DatA REAL);\n");
    ASSERT_FALSE(yyparse());
    auto table = getTable();
    Table expect_table("MyTablE", {{"DatA", DataType::real}});
    EXPECT_EQ(table, expect_table);
}

TEST(Parser_CreateTable, AnySpase) {
    setParserRequest(
        "create    \ntable a   (  \n  f  int   ,  h   real  , E   text )   ;   "
        "\n");
    ASSERT_FALSE(yyparse());
    auto table = getTable();
    Table expect_table("a", {{"f", DataType::integer},
                             {"h", DataType::real},
                             {"E", DataType::text}});
    EXPECT_EQ(table, expect_table);
}

TEST(Parser_CreateTable, EqDataType) {
    setParserRequest("create table a(b int, h int);\n");
    ASSERT_FALSE(yyparse());
    auto table = getTable();
    Table expect_table("a",
                       {{"b", DataType::integer}, {"h", DataType::integer}});
    EXPECT_EQ(table, expect_table);
}

TEST(Parser_CreateTable, WrongStyleComand) {
    setParserRequest("Create tAbLE MyTable(i Int);\n");
    ASSERT_TRUE(yyparse());
}

TEST(Parser_CreateTable, WrongDataType1) {
    setParserRequest("create table MyTable(create table MyTable);\n");
    ASSERT_TRUE(yyparse());
}

TEST(Parser_CreateTable, WrongDataType2) {
    setParserRequest("create table MyTable(Name rael);\n");
    ASSERT_TRUE(yyparse());
}

TEST(Parser_CreateTable, WrongParam1) {
    setParserRequest("create table MyTable();\n");
    ASSERT_TRUE(yyparse());
}

TEST(Parser_CreateTable, WrongParam2) {
    setParserRequest("create table MyTable(int);\n");
    ASSERT_TRUE(yyparse());
}

TEST(Parser_CreateTable, WrongParam3) {
    setParserRequest("create table MyTable(a);\n");
    ASSERT_TRUE(yyparse());
}

TEST(Parser_CreateTable, WrongParam4) {
    setParserRequest("create table MyTable(a int;);\n");
    ASSERT_TRUE(yyparse());
}

TEST(Parser_CreateTable, WithoutSemicolon) {
    setParserRequest("create table MyTable(a int)\n");
    ASSERT_TRUE(yyparse());
}

TEST(Parser_CreateTable, TwoCreate) {
    setParserRequest("create table MyTable1(a int); \n create table MyTable2(a int);\n");
    ASSERT_FALSE(yyparse());
}

TEST(Parser_CreateTable, WithOutPoints) {
    setParserRequest("create table MyTable(Name text)\n");
    ASSERT_TRUE(yyparse());
}

TEST(Parser_CreateTable, IntNameOfField) {
    setParserRequest("create table a(int int, h int);\n");
    ASSERT_TRUE(yyparse());
}

TEST(Parser_CreateTable, WithWalue) {
    setParserRequest("create table MyTable(Name text(5), Status int(2));\n");
    ASSERT_TRUE(yyparse());
}

TEST(Parser_ShowTable, SimpleTest) {
    setParserRequest("show table a;\n");
    ASSERT_FALSE(yyparse());
}

TEST(Parser_ShowTable, UPCASE) {
    setParserRequest("SHOW TABLE A;\n");
    ASSERT_FALSE(yyparse());
}

TEST(Parser_ShowTable, AnySpase) {
    setParserRequest("  \n  SHOW     TABLE  \n  A  ; \n");
    ASSERT_FALSE(yyparse());
}

TEST(Parser_ShowTable, WrongShow1) {
    setParserRequest("SHOW TABLE A(f int);\n");
    ASSERT_TRUE(yyparse());
}

TEST(Parser_ShowTable, WrongShow2) {
    setParserRequest("SHOW TABLE A();\n");
    ASSERT_TRUE(yyparse());
}
