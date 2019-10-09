#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../App/Core/Exception.h"
#include "../App/Our.h"
#include "Test.h"

TEST(CREATE_TABLE, TEST_1) {
    clearDB();
    CHECK_REQUEST("create table a (b int); show create table a;", 0,
                  "CREATE TABLE a(\n    b int\n);\n");
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
        "CREATE TABLE a(\n"
        "    b int not null,\n"
        "    c real unique,\n"
        "    d text primary key\n);\n");
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
        0,
        "CREATE TABLE a(\n    i int\n);\n"
        "CREATE TABLE b(\n    i int\n);\n"
        "CREATE TABLE c(\n    i int\n);\n");
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
        "create table a(b text not null unique primary key);"
        "show create table a;",
        0, "CREATE TABLE a(\n    b text primary key not null unique\n);\n");
}

TEST(CREATE_TABLE, TEST_13) {
    clearDB();
    CHECK_REQUEST("create table a(b int primary key, c int primary key);",
                  exc::ExceptionType::duplicated_primary_key,
                  "~~Exception 803 in table a:\n primary key is used in the "
                  "column b and in c.\n"
                  "~~Exception in command:\"create table a(b int primary key, "
                  "c int primary key);\"\n");
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
        0, "CREATE TABLE b(\n    c real\n);\n");
}

TEST(DROP_TABLE, TEST_3) {
    clearDB();
    CHECK_REQUEST("drop table;", exc::ExceptionType::syntax,
                  "~~Exception 1:\n wrong syntax!\n"
                  "~~Exception in command:\"drop table;\"\n");
}

TEST(SELECT, TEST_1) {
    clearDB();
    CHECK_REQUEST(
        "crate table a(a int, b real, c text);"
        "select * from a;",
        0, "");
}

TEST(SELECT, TEST_2) {
    clearDB();
    CHECK_REQUEST(
        "crate table a(a int, b int, c int);"
        "select *, c, a from a;",
        0, "");
}

TEST(SELECT, TEST_3) {
    clearDB();
    CHECK_REQUEST(
        "crate table a(a int, b real, c text);"
        "select c, * from a;",
        exc::ExceptionType::syntax,
        "~~Exception 1:\n wrong syntax!\n"
        "~~Exception in command:\"select c, * from a;\"\n");
}

TEST(SELECT, TEST_4) {
    clearDB();
    CHECK_REQUEST("select * from a;",
                  exc::ExceptionType::access_table_nonexistent, "");
}

TEST(SELECT, TEST_5) {
    clearDB();
    CHECK_REQUEST(
        "crate table a(a int, b real, c text);"
        "select *, c, a, f from a;",
        exc::ExceptionType::access_column_nonexistent, "");
}

TEST(SELECT, TEST_6) {
    clearDB();
    CHECK_REQUEST(
        "crate table a(a int, b real, c text);"
        "drop table a;"
        "select * from a;",
        exc::ExceptionType::access_table_nonexistent, "");
}

TEST(SELECT, TEST_7) {
    clearDB();
    CHECK_REQUEST(
        "crate table a(a int, b real, c text);"
        "insert into a values (1, 0, '1');"
        "insert into a values (1, 1, '0');"
        "insert into a values (0, 1, '1');"
        "select * from a where a = 1;"
        "select * from a where b = 0;"
        "select * from a where c = '1';",
        0, "");
}

TEST(INSERT, TEST_1) {
    clearDB();
    CHECK_REQUEST(
        "crate table a(a int, b real, c text);"
        "insert into a values (3, 2.2, 'Hello');"
        "select * from a;",
        0, "");
}

TEST(INSERT, TEST_2) {
    clearDB();
    CHECK_REQUEST(
        "crate table a(a int, b real, c text);"
        "insert into a (c, a, b) values ('Hello', 3, 2.2);"
        "select * from a;",
        0, "");
}

TEST(INSERT, TEST_3) {
    clearDB();
    CHECK_REQUEST(
        "crate table a(a int, b real, c text);"
        "insert into a values (2.2, 3.3, 'Hello');",
        0, "");  // TODO: ошибка несоответсвия типа данных
}

TEST(INSERT, TEST_4) {
    clearDB();
    CHECK_REQUEST(
        "crate table a(a int, b real, c text);"
        "insert into a values (-2, -1, '');"
        "select * from a;",
        0, "");
}

TEST(INSERT, TEST_5) {
    clearDB();
    CHECK_REQUEST(
        "crate table a(a int, b real, c text);"
        "insert into a values (2, 3.3.3, 'Hello');",
        0, "");  // TODO: ошибка несоответсвия типа данных
}

TEST(INSERT, TEST_6) {
    clearDB();
    CHECK_REQUEST(
        "crate table a(a int, b real, c text);"
        "insert into a values ('Hello', 3.3, 'Hello');",
        0, "");  // TODO: ошибка несоответсвия типа данных
}

TEST(INSERT, TEST_7) {
    clearDB();
    CHECK_REQUEST(
        "crate table a(a int, b real, c text);"
        "insert into a values (2, 3.3, 4);",
        0, "");  // TODO: ошибка несоответсвия типа данных
}

TEST(INSERT, TEST_8) {
    clearDB();
    CHECK_REQUEST(
        "crate table a(a int, b real, c text);"
        "insert into a(b, a, c) values (2, 3.3, 'Hello');",
        0, "");  // TODO: ошибка несоответсвия типа данных
}

TEST(INSERT, TEST_9) {
    clearDB();
    CHECK_REQUEST("insert into a values (2, 3.3, 'Hello');",
                  exc::ExceptionType::access_table_nonexistent, "");
}

TEST(INSERT, TEST_10) {
    clearDB();
    CHECK_REQUEST(
        "crate table a(a int, b real, c text);"
        "insert into a(a, b, f) values (2, 3.3, 'Hello');",
        exc::ExceptionType::access_column_nonexistent,
        "");  // TODO: ошибка несоответсвия типа данных
}

TEST(INSERT, TEST_11) {
    clearDB();
    CHECK_REQUEST(
        "crate table a(a int, b real, c text);"
        "insert into a(a) values (-2);"
        "select * from a;",
        0, "");
}

TEST(INSERT, TEST_12) {
    clearDB();
    CHECK_REQUEST(
        "crate table a(a int, b real, c text);"
        "insert into a(a) values ('H');"
        "select * from a;",
        0, "");  // TODO: ошибка несоответсвия типа данных
}

TEST(INSERT, TEST_13) {
    clearDB();
    CHECK_REQUEST(
        "crate table a(a int not null, b real primary key, c text unique);"
        "insert into a values (-2, 0.1, 'Hello world!');"
        "select * from a;",
        0, "");
}

TEST(INSERT, TEST_14) {
    clearDB();
    CHECK_REQUEST(
        "crate table a(a int not null, b real primary key, c text unique);"
        "insert into a(b, c) values (0.1, 'H M!');",
        0, "");  // TODO: ошибка нарушения констр
}

TEST(INSERT, TEST_15) {
    clearDB();
    CHECK_REQUEST(
        "crate table a(a int not null, b real primary key, c text unique);"
        "insert into a(a, c) values (1, 'H M!');",
        0, "");  // TODO: ошибка нарушения констр
}

TEST(INSERT, TEST_16) {
    clearDB();
    CHECK_REQUEST(
        "crate table a(a int not null, b real primary key, c text unique);"
        "insert into a values (1, 0, 'H M!');"
        "insert into a values (12, 0, 'H!');",
        0, "");  // TODO: ошибка нарушения констр
}

TEST(INSERT, TEST_17) {
    clearDB();
    CHECK_REQUEST(
        "crate table a(a int not null, b real primary key, c text unique);"
        "insert into a values (1, 0, 'H!');"
        "insert into a values (12, 1, 'H!');",
        0, "");  // TODO: ошибка нарушения констр
}

TEST(INSERT, TEST_18) {
    clearDB();
    CHECK_REQUEST(
        "crate table a(a int not null, b real primary key, c text unique);"
        "insert into a values (1, 0, 'H!');"
        "insert into a(a, b) values (12, 1);",
        0, "");  // TODO: ошибка нарушения констр
}

TEST(INSERT, TEST_19) {
    clearDB();
    CHECK_REQUEST(
        "crate table a(a int not null, b real primary key, c text unique);"
        "insert into a values (1, 0, 'H!');"
        "insert into a(a, b) values (12, 1);"
        "select * from a;",
        0, "");
}

TEST(INSERT, TEST_20) {
    clearDB();
    CHECK_REQUEST(
        "crate table a(a int, b real, c text);"
        "insert into a(a, a) values (12, 1);",
        0, "");  // TODO: ошибка дублирования полей
}

TEST(INSERT, TEST_21) {
    clearDB();
    CHECK_REQUEST(
        "crate table a(a int, b real, c text);"
        "insert into a(a) values (12, 1);",
        0, "");  // TODO: ошибка несоответствия количества полей со значениями
}

TEST(INSERT, TEST_22) {
    clearDB();
    CHECK_REQUEST(
        "crate table a(a int, b real, c text);"
        "insert into a values (12, 12, ' ') where b = 3;",
        exc::ExceptionType::syntax, "");
}

TEST(DELETE, TEST_1) {
    clearDB();
    CHECK_REQUEST(
        "crate table a(a int, b real, c text);"
        "insert into a values (1, 0, '1');"
        "insert into a values (1, 1, '0');"
        "insert into a values (0, 1, '1');"
        "delete a where a = 0;"
        "select * from a;"
        "delete a where b = 0;"
        "select * from a;"
        "delete a where c = '0'"
        "select * from a;",
        0, "");
}

TEST(DELETE, TEST_2) {
    clearDB();
    CHECK_REQUEST(
        "crate table a(a int, b real, c text);"
        "delete a where c = '0'"
        "select * from a;",
        0, "");
}

TEST(DELETE, TEST_3) {
    clearDB();
    CHECK_REQUEST("delete a where c = '0'",
                  exc::ExceptionType::access_table_nonexistent, "");
}

TEST(DELETE, TEST_4) {
    clearDB();
    CHECK_REQUEST(
        "crate table a(a int, b real, c text);"
        "delete a where f = '0'",
        exc::ExceptionType::access_table_nonexistent, "");
}