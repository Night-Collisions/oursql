#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../Test.h"
#include "parser.cpp"

extern Table getTable();

TEST(Parser_Empty, Semi) { ASSERT_TRUE(parse_string(";")); }

TEST(Parser_CreateTable, SimpleTest) {
    ASSERT_FALSE(parse_string("create table MyTable(Name text);\n"));
    auto table = getTable();
    Table expect_table("MyTable", {{"Name", DataType::text}});
    clearDB();
    EXPECT_EQ(table, expect_table);
}

TEST(Parser_CreateTable, UPCASE) {
    ASSERT_FALSE(parse_string("CREATE TABLE MyTablE(DatA REAL);\n"));
    auto table = getTable();
    Table expect_table("MyTablE", {{"DatA", DataType::real}});
    clearDB();
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
    clearDB();
    EXPECT_EQ(table, expect_table);
}

TEST(Parser_CreateTable, EqDataType) {
    ASSERT_FALSE(parse_string("create table a(b int, h int);\n"));
    auto table = getTable();
    Table expect_table("a",
                       {{"b", DataType::integer}, {"h", DataType::integer}});
    clearDB();
    EXPECT_EQ(table, expect_table);
}

TEST(Parser_CreateTable, WrongStyleComand) {
    ASSERT_FALSE(parse_string("Create tAbLE MyTable(i Int);\n"));
}

TEST(Parser_CreateTable, SameParam) {
    ASSERT_FALSE(parse_string("create table MyTable(a int, a text);\n"));
}

TEST(Parser_CreateTable, TwoPrimaryKey) {
    ASSERT_FALSE(parse_string(
        "create table MyTable(a int primary key, b text primary key);\n"));
}

TEST(Parser_CreateTable, TwoSamyContr) {
    ASSERT_FALSE(parse_string("create table MyTable(a int unique unique);\n"));
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

TEST(Parser_CreateTable, TypeGoesFirst) {
    ASSERT_TRUE(parse_string("create table MyTable(int a);\n"));
}

TEST(Parser_CreateTable, WrongParam3) {
    ASSERT_TRUE(parse_string("create table MyTable(a);\n"));
}

TEST(Parser_CreateTable, WrongParam4) {
    ASSERT_TRUE(parse_string("create table MyTable(a int;);\n"));
}

TEST(Parser_CreateTable, WrongParamName) {
    ASSERT_TRUE(parse_string("create table MyTable(1a int);\n"));
}

TEST(Parser_CreateTable, ParamName) {
    ASSERT_FALSE(parse_string("create table MyTable(a2b3 int);\n"));
}

TEST(Parser_CreateTable, WrongName) {
    ASSERT_TRUE(parse_string("create table 1MyTable(a int);\n"));
}

TEST(Parser_CreateTable, NameWithNumber) {
    ASSERT_FALSE(parse_string("create table My1Table2(a int);\n"));
}

TEST(Parser_CreateTable, WithoutSemicolon) {
    ASSERT_TRUE(parse_string("create table MyTable(a int)\n"));
}

TEST(Parser_CreateTable, TwoCreate) {
    ASSERT_FALSE(parse_string(
        "create table MyTable1(a int); \n create table MyTable2(a int);\n"));
}

TEST(Parser_CreateTable, WithOutPoints) {
    ASSERT_TRUE(parse_string("create table MyTable(Name text)\n"));
}

TEST(Parser_CreateTable, IntNameOfColumn) {
    ASSERT_TRUE(parse_string("create table a(int int, h int);\n"));
}

TEST(Parser_CreateTable, WithWalue) {
    ASSERT_TRUE(
        parse_string("create table MyTable(Name text(5), Status int(2));\n"));
}

TEST(Parser_CreateTable, Constraint) {
    ASSERT_FALSE(parse_string(
        "create table a(b int not null, h int primary key, d text unique);\n"));
    auto table = getTable();
    Table expect_table(
        "a", {{"b", DataType::integer, {ColumnConstraint::not_null}},
              {"h", DataType::integer, {ColumnConstraint::primary_key}},
              {"d", DataType::text, {ColumnConstraint::unique}}});
    clearDB();
    EXPECT_EQ(table, expect_table);
}

TEST(Parser_CreateTable, MultyConstraint) {
    ASSERT_FALSE(
        parse_string("create table a(b int not null unique primary key);\n"));
    auto table = getTable();
    Table expect_table(
        "a", {{"b",
               DataType::integer,
               {ColumnConstraint::not_null, ColumnConstraint::primary_key,
                ColumnConstraint::unique}}});
    clearDB();
    EXPECT_EQ(table, expect_table);
}

TEST(Parser_CreateTable, WrongMultyConstraint) {
    ASSERT_TRUE(
        parse_string("create table a(b int null primary not key unique);\n"));
}

TEST(Parser_CreateTable, WrongConstraint) {
    ASSERT_TRUE(parse_string("create table a(b int null);\n"));
}

TEST(Parser_CreateTable, MixedConstraint) {
    ASSERT_FALSE(parse_string(
        "create table a(b int, h int primary key unique, d text);\n"));
    auto table = getTable();
    Table expect_table(
        "a", {{"b", DataType::integer},
              {"h",
               DataType::integer,
               {ColumnConstraint::primary_key, ColumnConstraint::unique}},
              {"d", DataType::text}});
    clearDB();
    EXPECT_EQ(table, expect_table);
}

TEST(Parser_DropTable, SimpleTest) {
    ASSERT_FALSE(parse_string("drop table a;\n"));
}

TEST(Parser_DropTable, UPCASE) {
    ASSERT_FALSE(parse_string("DROP TABLE a;\n"));
}

TEST(Parser_DropTable, WrongDrop1) {
    ASSERT_TRUE(parse_string("drop table a();\n"));
}

TEST(Parser_DropTable, WrongDrop2) {
    ASSERT_TRUE(parse_string("drop table a(b int);\n"));
}

TEST(Parser_DropTable, WrongDrop3) {
    ASSERT_TRUE(parse_string("drop table;\n"));
}

TEST(Parser_DropTable, WrongDrop4) {
    ASSERT_TRUE(parse_string("drop table drop;\n"));
}

TEST(Parser_DropTable, WrongDrop5) {
    ASSERT_TRUE(parse_string("drop table 1Adrop;\n"));
}

TEST(Parser_ShowCreateTable, SimpleTest) {
    ASSERT_FALSE(parse_string("show create table A;\n"));
}

TEST(Parser_ShowCreateTable, Wrong1) {
    ASSERT_TRUE(parse_string("create table show A;\n"));
}

TEST(Parser_ShowCreateTable, Wrong2) {
    ASSERT_TRUE(parse_string("show create table 1A;\n"));
}

TEST(Parser_ShowCreateTable, AnyCase) {
    ASSERT_FALSE(parse_string("Show creaTe tAble A;\n"));
}

TEST(Parser_ShowCreateTable, IncorrectKeyWord) {
    ASSERT_TRUE(parse_string("Show creaTe tAbles A;\n"));
}

TEST(Parser_ShowCreateTable, VarName) {
    ASSERT_FALSE(parse_string("create table a(null int);\n"));
}

TEST(Parser_ShowCreateTable, ConstraintAsName1) {
    ASSERT_TRUE(parse_string("create table a(not null int);\n"));
}

TEST(Parser_ShowCreateTable, ConstraintAsName2) {
    ASSERT_TRUE(parse_string("create table a(unique int);\n"));
}

TEST(Parser_ShowCreateTable, ConstraintAsName3) {
    ASSERT_TRUE(parse_string("create table a(primary key int);\n"));
}
