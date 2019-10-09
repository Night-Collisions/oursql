#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../App/Core/Exception.h"
#include "../App/Our.h"
#include "Test.h"

TEST(CREATE_TABLE, TEST_1) {
    clearDB();
    CHECK_REQUEST("create table a (b int); show create table a;", 0, "");
}

TEST(CREATE_TABLE, TEST_2) {
    clearDB();
    CHECK_REQUEST("create table a (b int)", exc::ExceptionType::syntax,
                  "~~Exception 1:\n wrong syntax!\n~~Exception in "
                  "command:\"create table a (b int)\"\n");
}

TEST(CREATE_TABLE, TEST_3) {
    clearDB();
    CHECK_REQUEST("create table a(b int not null);", 0, "");
}

TEST(CREATE_TABLE, TEST_4) {
    clearDB();
    CHECK_REQUEST("create table a(b int not null not null);",
                  exc::ExceptionType::redundant_constraints,
                  "~~Exception 802:\n duplicate of constraint b in column a.\n"
                  "~~Exception in command:\"create table a(b int not null not "
                  "null);\"\n");
}

TEST(CREATE_TABLE, TEST_5) {
    clearDB();
    CHECK_REQUEST(
        "create table a(b int not null, c real unique, d text primary key);"
        "show create table a;",
        0,
        "CREATE TABLE\n"
        "a(\n    b int not null,\n    c real unique,\n    d text primary "
        "key\n);");
}

TEST(CREATE_TABLE, TEST_6) {
    clearDB();
    CHECK_REQUEST("create table sfs;", exc::ExceptionType::syntax,
                  "~~Exception 1:\n wrong syntax!\n"
                  "~~Exception in command:\"create table sfs;\"\n");
}

TEST(CREATE_TABLE, TEST_7) {
    clearDB();
    CHECK_REQUEST(
        "create table a(i int); create table b(i int); create table c(i int); "
        "show create table a; show create table b; show create table c;",
        0, "");
}

TEST(CREATE_TABLE, TEST_8) {
    clearDB();
    CHECK_REQUEST("create table a(MyColumn int, mycolumn text);",
                  exc::ExceptionType::repeat_column_in_table,
                  "~~Exception 3:\n repeat column MyColumn in table a.\n"
                  "~~Exception in command:\"create table a(MyColumn int, "
                  "mycolumn text);\"\n");
}

TEST(CREATE_TABLE, TEST_9) {
    clearDB();
    CHECK_REQUEST(
        "create table a(MyColumn int, q text); create table a(f int);",
        exc::ExceptionType::create_table_repeat_table_name,
        "~~Exception 1002 in create table a:\n this table name is repeated!\n"
        "~~Exception in command:\" create table a(f int);\"\n");
}

TEST(CREATE_TABLE, TEST_10) {
    clearDB();
    CHECK_REQUEST("create table 1a(f int);", exc::ExceptionType::syntax,
                  "~~Exception 1:\n wrong syntax!\n"
                  "~~Exception in command:\"create table 1a(f int);\"\n");
}

TEST(CREATE_TABLE, TEST_11) {
    clearDB();
    CHECK_REQUEST("create table a(1f int);", exc::ExceptionType::syntax,
                  "~~Exception 1:\n wrong syntax!\n"
                  "~~Exception in command:\"create table a(1f int);\"\n");
}

TEST(CREATE_TABLE, TEST_12) {
    clearDB();
    CHECK_REQUEST(
        "create table a(b text not null unique primary key); show create table "
        "a;",
        0, "");
}

TEST(CREATE_TABLE, TEST_13) {
    clearDB();
    CHECK_REQUEST("create table a(b int primary key, c int primary key);",
                  exc::ExceptionType::duplicated_primary_key, "");
}

TEST(SHOW_CREATE_TABLE, TEST_1) {
    clearDB();
    CHECK_REQUEST("show ghgh;", exc::ExceptionType::syntax,
                  "~~Exception 1:\n wrong syntax!\n"
                  "~~Exception in command:\"show ghgh;\"\n");
}

TEST(SHOW_CREATE_TABLE, TEST_2) {
    clearDB();
    CHECK_REQUEST("show create table e;",
                  exc::ExceptionType::access_table_nonexistent,
                  "~~Exception 701:\n table e nonexistent.\n~~Exception in "
                  "command:\"show create table e;\"\n");
}

TEST(SYNTAX, TEST_1) {
    clearDB();
    CHECK_REQUEST("CreAte    \n  TablE   NamE \n ( A ReAl);", 0, "");
}

TEST(SYNTAX, TEST_2) {
    clearDB();
    CHECK_REQUEST(
        "fdgd;", exc::ExceptionType::syntax,
        "~~Exception 1:\n wrong syntax!\n~~Exception in command:\"fdgd;\"\n");
}

TEST(SYNTAX, TEST_3) {
    clearDB();
    CHECK_REQUEST("CREATE TABLE TAB(F TEXT);", 0, "");
}

TEST(DROP_TABLE, TEST_1) {
    clearDB();
    CHECK_REQUEST("drop table tr;",
                  exc::ExceptionType::access_table_nonexistent,
                  "~~Exception 701:\n table tr nonexistent."
                  "\n~~Exception in command:\"drop table tr;\"\n");
}

TEST(DROP_TABLE, TEST_2) {
    clearDB();
    CHECK_REQUEST(
        "create table a(b int); create table b(c real); drop table a; create "
        "table a(c text); show create table b;",
        0, "");
}

TEST(DROP_TABLE, TEST_3) {
    clearDB();
    CHECK_REQUEST("drop table;", exc::ExceptionType::syntax,
                  "~~Exception 1:\n wrong syntax!\n"
                  "~~Exception in command:\"drop table;\"\n");
}
