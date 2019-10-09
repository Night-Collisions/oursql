#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../../App/Core/Exception.h"
#include "../../App/Our.h"
#include "Test.h"

#define NO_EXCEPTION(exception) ASSERT_EQ(exception, 0);
#define CHECK_EXCEPTION(exception, excpect) \
    ASSERT_EQ(exception, static_cast<unsigned int>(excpect));

#define CHECK_REQUEST(request, exception, answer)         \
    std::stringstream in(request);                        \
    std::stringstream out;                                \
    CHECK_EXCEPTION(ourSQL::perform(in, out), exception); \
    EXPECT_EQ(out.str(), answer);

TEST(CREATE_TABLE, TEST_1) {
    CHECK_REQUEST("create table a (b int); show create table a;", 0, "");
    clearDB();
}

TEST(CREATE_TABLE, TEST_2) {
    CHECK_REQUEST("create table a (b int)", exc::ExceptionType::syntax,
                  "~~Exception 3: wrong syntax!\n~~Exception in "
                  "command:\"create table a (b int)\"\n");
    clearDB();
}

TEST(CREATE_TABLE, TEST_3) {
    CHECK_REQUEST("create table a(b int not null);", 0, "");
    clearDB();
}

TEST(CREATE_TABLE, TEST_4) {
    CHECK_REQUEST("create table a(b int not null not null);",
                  exc::ExceptionType::redundant_constraints,
                  "~~Exception 802: duplicate of constraint b in column a.\n"
                  "~~Exception in command:\"create table a(b int not null not "
                  "null);\"\n");
    clearDB();
}

TEST(CREATE_TABLE, TEST_5) {
    CHECK_REQUEST(
        "create table a(b int not null, c real unique, d text primary key);"
        "show create table a;",
        0,
        "CREATE TABLE\n"
        "a(\n    b int not null,\n    c real unique,\n    d text primary "
        "key\n);");
    clearDB();
}

TEST(CREATE_TABLE, TEST_6) {
    CHECK_REQUEST("create table sfs;", exc::ExceptionType::syntax,
                  "~~Exception 3: wrong syntax!\n"
                  "~~Exception in command:\"create table sfs;\"\n");
    clearDB();
}

TEST(CREATE_TABLE, TEST_7) {
    CHECK_REQUEST(
        "create table a(i int); create table b(i int); create table c(i int); "
        "show create table a; show create table b; show create table c;",
        0, "");
    clearDB();
}

TEST(CREATE_TABLE, TEST_8) {
    CHECK_REQUEST("create table a(MyColumn int, mycolumn text);",
                  exc::ExceptionType::repeat_column_in_table,
                  "~~Exception 2: repeat column MyColumn in table a.\n"
                  "~~Exception in command:\"create table a(MyColumn int, mycolumn text);\"\n");
    clearDB();
}

TEST(CREATE_TABLE, TEST_9) {
    CHECK_REQUEST(
        "create table a(MyColumn int, q text); create table a(f int);",
        exc::ExceptionType::create_table_repeat_table_name, "");
    clearDB();
}

TEST(CREATE_TABLE, TEST_10) {
    CHECK_REQUEST("create table 1a(f int);",
                  exc::ExceptionType::create_table_name_table, "");
    clearDB();
}

TEST(CREATE_TABLE, TEST_11) {
    CHECK_REQUEST("create table a(_f int);",
                  exc::ExceptionType::create_table_name_column, "");
    clearDB();
}

TEST(CREATE_TABLE, TEST_12) {
    CHECK_REQUEST(
        "create table a(b text not null unique primary key); show create table "
        "a;",
        0, "");
    clearDB();
}

TEST(CREATE_TABLE, TEST_13) {
    CHECK_REQUEST("create table a(b primary key, c primary key);",
                  exc::ExceptionType::duplicated_primary_key, "");
    clearDB();
}

TEST(SHOW_CREATE_TABLE, TEST_1) {
    CHECK_REQUEST("show ghgh;", exc::ExceptionType::syntax, "");
    clearDB();
}

TEST(SHOW_CREATE_TABLE, TEST_2) {
    CHECK_REQUEST("show create table e;",
                  exc::ExceptionType::access_table_nonexistent, "");
    clearDB();
}

TEST(SYNTAX, TEST_1) {
    CHECK_REQUEST("CreAte    \n  TablE   NamE \n ( A ReAl);", 0, "");
    clearDB();
}

TEST(SYNTAX, TEST_2) {
    CHECK_REQUEST("fdgd;", exc::ExceptionType::syntax, "");
    clearDB();
}

TEST(SYNTAX, TEST_3) {
    CHECK_REQUEST("CREATE TABLE TAB(F TEXT);", 0, "");
    clearDB();
}

TEST(DROP_TABLE, TEST_1) {
    CHECK_REQUEST("drop table tr;",
                  exc::ExceptionType::access_table_nonexistent, "");
    clearDB();
}

TEST(DROP_TABLE, TEST_2) {
    CHECK_REQUEST(
        "create table a(b int); create table b(c real); drop table a; create "
        "table a(c text); show create table b;",
        0, "");
    clearDB();
}

TEST(DROP_TABLE, TEST_3) {
    CHECK_REQUEST("drop table;;", exc::ExceptionType::syntax, "");
    clearDB();
}
