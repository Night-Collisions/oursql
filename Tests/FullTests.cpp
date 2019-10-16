#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../Server/Core/Exception.h"
#include "../Server/Our.h"
#include "Test.h"

TEST(CREATE_TABLE, TEST_1) {
    clearDB();
    check_requests(
        {{"create table a (b int);", 0, ""},
         {"show create table a;", 0, "CREATE TABLE a(\n    b int\n);\n"}});
}

TEST(CREATE_TABLE, TEST_2) {
    clearDB();
    check_requests(
        {{"create table a (b int)", EXCEPTION2NUMB(exc::ExceptionType::syntax),
          "~~Exception 1:\n wrong syntax!\n~~Exception in command:\"create "
          "table a (b int)\"\n"}});
}

TEST(CREATE_TABLE, TEST_3) {
    clearDB();
    check_requests({{"create table a(b int not null);", 0, ""}});
}

TEST(CREATE_TABLE, TEST_4) {
    clearDB();
    check_requests(
        {{"create table a(b int not null not null);",
          EXCEPTION2NUMB(exc::ExceptionType::redundant_constraints),
          "~~Exception 802:\n duplicate of constraint b in column a.\n"
          "~~Exception in command:\"create table a(b int not null not "
          "null);\"\n"}});
}

TEST(CREATE_TABLE, TEST_5) {
    clearDB();
    check_requests(
        {{"create table a(b int not null, c real unique, d text primary key);",
          0, ""},
         {"show create table a;", 0,
          "CREATE TABLE a(\n"
          "    b int not null,\n"
          "    c real unique,\n"
          "    d text primary key\n);\n"}});
}

TEST(CREATE_TABLE, TEST_6) {
    clearDB();
    check_requests(
        {{"create table sfs;", EXCEPTION2NUMB(exc::ExceptionType::syntax),
          "~~Exception 1:\n wrong syntax!\n"
          "~~Exception in command:\"create table sfs;\"\n"}});
}

TEST(CREATE_TABLE, TEST_7) {
    clearDB();
    check_requests(
        {{"create table a(i int);", 0, ""},
         {"create table b(i int);", 0, ""},
         {"create table c(i int);", 0, ""},
         {"show create table a;", 0, "CREATE TABLE a(\n    i int\n);\n"},
         {"show create table b;", 0, "CREATE TABLE b(\n    i int\n);\n"},
         {"show create table c;", 0, "CREATE TABLE c(\n    i int\n);\n"}});
}

TEST(CREATE_TABLE, TEST_8) {
    clearDB();
    check_requests({{"create table a(MyColumn int, mycolumn text);",
                     EXCEPTION2NUMB(exc::ExceptionType::repeat_column_in_table),
                     "~~Exception 5:\n repeat column MyColumn in table a.\n"
                     "~~Exception in command:\"create table a(MyColumn int, "
                     "mycolumn text);\"\n"}});
}

TEST(CREATE_TABLE, TEST_9) {
    clearDB();
    check_requests(
        {{"create table a(MyColumn int, q text);", 0, ""},
         {"create table a(f int);",
          EXCEPTION2NUMB(exc::ExceptionType::create_table_repeat_table_name),
          "~~Exception 1002 in create table a:\n this table name is "
          "repeated!\n~~Exception in command:\"create table a(f int);\"\n"}});
}

TEST(CREATE_TABLE, TEST_10) {
    clearDB();
    check_requests(
        {{"create table 1a(f int);", EXCEPTION2NUMB(exc::ExceptionType::syntax),
          "~~Exception 1:\n wrong syntax!\n"
          "~~Exception in command:\"create table 1a(f int);\"\n"}});
}

TEST(CREATE_TABLE, TEST_11) {
    clearDB();
    check_requests(
        {{"create table a(1f int);", EXCEPTION2NUMB(exc::ExceptionType::syntax),
          "~~Exception 1:\n wrong syntax!\n"
          "~~Exception in command:\"create table a(1f int);\"\n"}});
}

TEST(CREATE_TABLE, TEST_12) {
    clearDB();
    check_requests(
        {{"create table a(b text not null unique primary key);", 0, ""},
         {"show create table a;", 0,
          "CREATE TABLE a(\n    b text primary key not null unique\n);\n"}});
}

TEST(CREATE_TABLE, TEST_13) {
    clearDB();
    check_requests(
        {{"create table a(b int primary key, c int primary key);",
          EXCEPTION2NUMB(exc::ExceptionType::duplicated_primary_key_in_column),
          "~~Exception 803 in table a:\n primary key is used in the column b "
          "and in c.\n"
          "~~Exception in command:\"create table a(b int primary key, c int "
          "primary key);\"\n"}});
}

TEST(SHOW_CREATE_TABLE, TEST_1) {
    clearDB();
    check_requests({{"show ghgh;", EXCEPTION2NUMB(exc::ExceptionType::syntax),
                     "~~Exception 1:\n wrong syntax!\n"
                     "~~Exception in command:\"show ghgh;\"\n"}});
}

TEST(SHOW_CREATE_TABLE, TEST_2) {
    clearDB();
    check_requests(
        {{"show create table e;",
          EXCEPTION2NUMB(exc::ExceptionType::access_table_nonexistent),
          "\n~~Exception 701:\n table e nonexistent.\n~~Exception in "
          "command:\"show create table e;\"\n"}});
}

TEST(SYNTAX, TEST_1) {
    clearDB();
    check_requests({{"CreAte    \n  TablE   NamE \n ( A ReAl);", 0, ""}});
}

TEST(SYNTAX, TEST_2) {
    clearDB();
    check_requests({{"fdgd;", EXCEPTION2NUMB(exc::ExceptionType::syntax),
                     "~~Exception 1:\n wrong syntax!\n~~Exception in "
                     "command:\"fdgd;\"\n"}});
}

TEST(SYNTAX, TEST_3) {
    clearDB();
    check_requests({{"CREATE TABLE TAB(F TEXT);", 0, ""}});
}

TEST(DROP_TABLE, TEST_1) {
    clearDB();
    check_requests(
        {{"drop table tr;",
          EXCEPTION2NUMB(exc::ExceptionType::access_table_nonexistent),
          "~~Exception 701:\n table tr nonexistent."
          "\n~~Exception in command:\"drop table tr;\"\n"}});
}

TEST(DROP_TABLE, TEST_2) {
    clearDB();
    check_requests(
        {{"create table a(b int);", 0, ""},
         {"create table b(c real);", 0, ""},
         {"drop table a;", 0, ""},
         {"create table a(c text);", 0, ""},
         {"show create table b;", 0, "CREATE TABLE b(\n    c real\n);\n"}});
}

TEST(DROP_TABLE, TEST_3) {
    clearDB();
    check_requests({{"drop table;", EXCEPTION2NUMB(exc::ExceptionType::syntax),
                     "~~Exception 1:\n wrong syntax!\n"
                     "~~Exception in command:\"drop table;\"\n"}});
}

TEST(SELECT, TEST_1) {
    clearDB();
    check_requests({{"create table a(a int, b real, c text);", 0, ""},
                    {"select * from a;", 0, ""}});
}

TEST(SELECT, TEST_2) {
    clearDB();
    check_requests({{"create table a(a int, b int, c int);", 0, ""},
                    {"select *, c, a from a;", 0, ""}});
}

TEST(SELECT, TEST_3) {
    clearDB();
    check_requests(
        {{"create table a(a int, b real, c text);", 0, ""},
         {"select c, * from a;", EXCEPTION2NUMB(exc::ExceptionType::syntax),
          "~~Exception 1:\n wrong syntax!\n"
          "~~Exception in command:\"select c, * from a;\"\n"}});
}

TEST(SELECT, TEST_4) {
    clearDB();
    check_requests(
        {{"select * from a;",
          EXCEPTION2NUMB(exc::ExceptionType::access_table_nonexistent),
          "~~Exception 701:\n table a nonexistent.\n"
          "~~Exception in command:\"select * from a;\"\n"}});
}

TEST(SELECT, TEST_5) {
    clearDB();
    check_requests(
        {{"create table a(a int, b real, c text);", 0, ""},
         {"select *, c, a, f from a;",
          EXCEPTION2NUMB(exc::ExceptionType::access_column_nonexistent),
          "~~Exception 702:\n column f in table a nonexistent.\n"
          "~~Exception in command:\"select *, c, a, f from a;\"\n"}});
}

TEST(SELECT, TEST_6) {
    clearDB();
    check_requests(
        {{"create table a(a int, b real, c text);", 0, ""},
         {"drop table a;", 0, ""},
         {"select * from a;",
          EXCEPTION2NUMB(exc::ExceptionType::access_table_nonexistent),
          "~~Exception 701:\n table a nonexistent.\n"
          "~~Exception in command:\"select * from a;\"\n"}});
}

TEST(SELECT, TEST_7) {
    clearDB();
    check_requests(
        {{"create table a(a int, b real, c text);", 0, ""},
         {"insert into a values (1, 0, '1');", 0, ""},
         {"insert into a values (1, 1, '0');", 0, ""},
         {"insert into a values (0, 1, '1');", 0, ""},
         {"select * from a where a = 0;", 0, "a: 0\nb: 1\nc: '1'\n"}});
}

TEST(SELECT, TEST_8) {
    clearDB();
    check_requests(
        {{"create table a(a int);", 0, ""},
         {"insert into a values (1);", 0, ""},
         {"select * from a where a = '1';",
          EXCEPTION2NUMB(exc::ExceptionType::compare_data_type_mismatch),
          "~~Exception 602:\n can't compare int and text.\n"
          "~~Exception in command:\"select * from a where a = '1';\"\n"}});
}

TEST(INSERT, TEST_1) {
    clearDB();
    check_requests({{"create table a(a int, b real, c text);", 0, ""},
                    {"insert into a values (3, 2.2, 'Hello');", 0, ""},
                    {"select * from a;", 0, "a: 3\nb: 2.2\nc: 'Hello'\n"}});
}

TEST(INSERT, TEST_2) {
    clearDB();
    check_requests(
        {{"create table a(a int, b real, c text);", 0, ""},
         {"insert into a (c, a, b) values ('Hello', 3, 2.2);", 0, ""},
         {"select * from a;", 0, "a: 3\nb: 2.2\nc: 'Hello'\n"}});
}

TEST(INSERT, TEST_3) {
    clearDB();
    check_requests(
        {{"create table a(a int, b real, c text);", 0, ""},
         {"insert into a values (2.2, 3.3, 'Hello');",
          EXCEPTION2NUMB(exc::ExceptionType::set_data_type_mismatch),
          "~~Exception 601:\n value a is not compatible with data type int.\n"
          "~~Exception in command:\"insert into a values (2.2, 3.3, "
          "'Hello');\"\n"}});
}

TEST(INSERT, TEST_4) {
    clearDB();
    check_requests({{"create table a(a int, b real, c text);", 0, ""},
                    {"insert into a values (-2, -1, '');", 0, ""},
                    {"select * from a;", 0, "a: -2\nb: -1\nc: ''\n"}});
}

TEST(INSERT, TEST_5) {
    clearDB();
    check_requests({{"create table a(a int, b real, c text);", 0, ""},
                    {"insert into a values (2, 3.3.3, 'Hello');",
                     EXCEPTION2NUMB(exc::ExceptionType::syntax),
                     "~~Exception 1:\n wrong syntax!\n"
                     "~~Exception in command:\"insert into a values (2, 3.3.3, "
                     "'Hello');\"\n"}});
}

TEST(INSERT, TEST_6) {
    clearDB();
    check_requests(
        {{"create table a(a int, b real, c text);", 0, ""},
         {"insert into a values ('Hello', 3.3, 'Hello');",
          EXCEPTION2NUMB(exc::ExceptionType::set_data_type_mismatch),
          "~~Exception 601:\n value a is not compatible with data type int.\n"
          "~~Exception in command:\"insert into a values ('Hello', 3.3, "
          "'Hello');\"\n"}});
}

TEST(INSERT, TEST_7) {
    clearDB();
    check_requests(
        {{"create table a(a int, b real, c text);", 0, ""},
         {"insert into a values (2, 3.3, 4);",
          EXCEPTION2NUMB(exc::ExceptionType::set_data_type_mismatch),
          "~~Exception 601:\n value c is not compatible with data type text.\n"
          "~~Exception in command:\"insert into a values (2, 3.3, 4);\"\n"}});
}

TEST(INSERT, TEST_8) {
    clearDB();
    check_requests(
        {{"create table a(a int, b real, c text);", 0, ""},
         {"insert into a(b, a, c) values (2, 3.3, 'Hello');",
          EXCEPTION2NUMB(exc::ExceptionType::set_data_type_mismatch),
          "~~Exception 601:\n value a is not compatible with data type "
          "int.\n~~Exception in command:\"insert into a(b, a, c) values (2, "
          "3.3, "
          "'Hello');\"\n"}});
}

TEST(INSERT, TEST_9) {
    clearDB();
    check_requests(
        {{"insert into a values (2, 3.3, 'Hello');",
          EXCEPTION2NUMB(exc::ExceptionType::access_table_nonexistent),
          "~~Exception 701:\n table a nonexistent.\n~~Exception in "
          "command:\"insert into a values (2, 3.3, 'Hello');\"\n"}});
}

TEST(INSERT, TEST_10) {
    clearDB();
    check_requests(
        {{"create table a(a int, b real, c text);", 0, ""},
         {"insert into a(a, b, f) values (2, 3.3, 'Hello');",
          EXCEPTION2NUMB(exc::ExceptionType::access_column_nonexistent),
          "~~Exception 702:\n column f in table a nonexistent.\n"
          "~~Exception in command:\"insert into a(a, b, f) values (2, 3.3, "
          "'Hello');\"\n"}});
}

TEST(INSERT, TEST_11) {
    clearDB();
    check_requests({{"create table a(a int, b real, c text);", 0, ""},
                    {"insert into a(a) values (-2);", 0, ""},
                    {"select * from a;", 0, "a: -2\nb: null\nc: null\n"}});
}

TEST(INSERT, TEST_12) {
    clearDB();
    check_requests({{"create table a(a int, b real, c text);", 0, ""},
                    {"insert into a(a) values (8);", 0, ""},
                    {"select * from a;", 0, "a: 8\nb: null\nc: null\n"}});
}

TEST(INSERT, TEST_13) {
    clearDB();
    check_requests(
        {{"create table a(a int not null, b real primary key, c text unique);",
          0, ""},
         {"insert into a values (-2, 0.1, 'Hello world!');", 0, ""},
         {"select * from a;", 0, "a: -2\nb: 0.1\nc: 'Hello world!'\n"}});
}

TEST(INSERT, TEST_14) {
    clearDB();
    check_requests(
        {{"create table a(a int not null, b real primary key, c text unique);",
          0, ""},
         {"insert into a(b, c) values (0.1, 'H M!');",
          EXCEPTION2NUMB(exc::ExceptionType::null_not_null),
          "~~Exception 805 in table a:\n a can't contain null values.\n"
          "~~Exception in command:\"insert into a(b, c) values (0.1, 'H "
          "M!');\"\n"}});
}

TEST(INSERT, TEST_15) {
    clearDB();
    check_requests(
        {{"create table a(a int not null, b real primary key, c text unique);",
          0, ""},
         {"insert into a values (1, 0, 'H M!');", 0, ""},
         {"insert into a values (12, 0, 'H!');",
          EXCEPTION2NUMB(exc::ExceptionType::duplicated_unique),
          "~~Exception 804 in table a:\n 0 is not unique is in the column "
          "b.\n~~Exception in command:\"insert into a values (12, 0, "
          "'H!');\"\n"}});
}

TEST(INSERT, TEST_16) {
    clearDB();
    check_requests(
        {{"create table a(a int not null, b real primary key, c text unique);",
          0, ""},
         {"insert into a values (1, 0, 'H!');", 0, ""},
         {"insert into a values (12, 1, 'H!');",
          EXCEPTION2NUMB(exc::ExceptionType::duplicated_unique),
          "~~Exception 804 in table a:\n 'H!' is not unique is in the column "
          "c.\n~~Exception in command:\"insert into a values (12, 1, "
          "'H!');\"\n"}});
}

TEST(INSERT, TEST_17) {
    clearDB();
    check_requests(
        {{"create table a(a int not null, b real primary key, c text unique);",
          0, ""},
         {"insert into a values (1, 0, null);", 0, ""},
         {"insert into a(a, b) values (12, 1);",
          EXCEPTION2NUMB(exc::ExceptionType::duplicated_unique),
          "~~Exception 804 in table a:\n null is not unique is in the column "
          "c.\n~~Exception in command:\"insert into a(a, b) values (12, "
          "1);\"\n"}});
}

TEST(INSERT, TEST_18) {
    clearDB();
    check_requests(
        {{"create table a(a int not null, b real primary key, c text unique);",
          0, ""},
         {"insert into a values (1, 0, 'H!');", 0, ""},
         {"insert into a(a, b) values (12, 1);"
          "select * from a;",
          0, "a: 1\nb: 0\nc: 'H!'\na: 12\nb: 1\nc: null\n"}});
}

TEST(INSERT, TEST_19) {
    clearDB();
    check_requests({{"create table a(a int, b real, c text);", 0, ""},
                    {"insert into a(a, a) values (12, 1);",
                     EXCEPTION2NUMB(exc::ExceptionType::repeat_column),
                     "~~Exception 6:\n repeat column a.\n~~Exception in "
                     "command:\"insert "
                     "into a(a, a) values (12, 1);\"\n"}});
}

TEST(INSERT, TEST_20) {
    clearDB();
    check_requests(
        {{"create table a(a int, b real, c text);", 0, ""},
         {"insert into a(a) values (12, 1);",
          EXCEPTION2NUMB(exc::ExceptionType::insert_constants_more_columns),
          "~~Exception 1101:\n the number of constants is more than "
          "columns.\n"
          "~~Exception in command:\"insert into a(a) values (12, 1);\"\n"}});
}

TEST(INSERT, TEST_21) {
    clearDB();
    check_requests(
        {{"create table a(a int, b real, c text);", 0, ""},
         {"insert into a values (12, 12, ' ') where b = 3;",
          EXCEPTION2NUMB(exc::ExceptionType::syntax),
          "~~Exception 1:\n wrong syntax!\n"
          "~~Exception in command:\"insert into a values (12, 12, ' ') where "
          "b = "
          "3;\"\n"}});
}

TEST(DELETE, TEST_1) {
    clearDB();
    check_requests(
        {{"create table a(a int, b real, c text);", 0, ""},
         {"insert into a values (1, 0, '1');", 0, ""},
         {"insert into a values (1, 1, '0');", 0, ""},
         {"insert into a values (0, 1, '1');", 0, ""},
         {"delete from a where a = 0;", 0, ""},
         {"select * from a;", 0, "a: 1\nb: 0\nc: '1'\na: 1\nb: 1\nc: '0'\n"},
         {"delete from a where b = 0;", 0, ""},
         {"select * from a;", 0, "a: 1\nb: 1\nc: '0'\n"},
         {"delete from a where c = '0';", 0, ""},
         {"select * from a;", 0, ""}});
}

TEST(DELETE, TEST_2) {
    clearDB();
    check_requests({{"create table a(a int, b real, c text);", 0, ""},
                    {"delete from a where c = '0';", 0, ""},
                    {"select * from a;", 0, ""}});
}

TEST(DELETE, TEST_3) {
    clearDB();
    check_requests(
        {{"delete from a where c = '0';",
          EXCEPTION2NUMB(exc::ExceptionType::access_table_nonexistent),
          "~~Exception 701:\n table a nonexistent.\n~~Exception in "
          "command:\"delete from a where c = '0';\"\n"}});
}

TEST(DELETE, TEST_4) {
    clearDB();
    check_requests(
        {{"create table a(a int, b real, c text);", 0, ""},
         {"delete from a where f = '0';",
          EXCEPTION2NUMB(exc::ExceptionType::access_column_nonexistent),
          "~~Exception 702:\n column f in table a nonexistent.\n~~Exception "
          "in "
          "command:\"delete from a where f = '0';\"\n"}});
}

TEST(DELETE, TEST_5) {
    clearDB();
    check_requests(
        {{"create table a(a int, b real, c text);", 0, ""},
         {"delete from a where b = '0';",
          EXCEPTION2NUMB(exc::ExceptionType::compare_data_type_mismatch),
          "~~Exception 602:\n can't compare real and text.\n~~Exception in "
          "command:\"delete from a where b = '0';\"\n"}});
}

TEST(UPDATE, TEST_1) {
    clearDB();
    check_requests(
        {{"create table a(a int, b real, c text);", 0, ""},
         {"insert into a values (1, 0, '1');", 0, ""},
         {"insert into a values (1, 1, '0');", 0, ""},
         {"insert into a values (0, 1, '1');", 0, ""},
         {"update a set a = 2;", 0, ""},
         {"select * from a;", 0,
          "a: 2\nb: 0\nc: '1'\na: 2\nb: 1\nc: '0'\na: 2\nb: 1\nc: '1'\n"},
         {"update a set b = 3.45, c = 'H';", 0, ""},
         {"select * from a;", 0,
          "a: 2\nb: 3.45\nc: 'H'\na: 2\nb: 3.45\nc: 'H'\na: 2\nb: 3.45\nc: "
          "'H'\n"}});
}

TEST(UPDATE, TEST_2) {
    clearDB();
    check_requests(
        {{"create table a(a int, b real, c text);", 0, ""},
         {"insert into a values (1, 0, '1');", 0, ""},
         {"insert into a values (1, 1, '0');", 0, ""},
         {"insert into a values (0, 1, '1');", 0, ""},
         {"update a set f = 2;",
          EXCEPTION2NUMB(exc::ExceptionType::access_column_nonexistent),
          "~~Exception 702:\n column f in table a nonexistent.\n~~Exception "
          "in "
          "command:\"update a set f = 2;\"\n"}});
}

TEST(UPDATE, TEST_3) {
    clearDB();
    check_requests(
        {{"update a set f = 2;",
          EXCEPTION2NUMB(exc::ExceptionType::access_table_nonexistent),
          "~~Exception 701:\n table a nonexistent.\n~~Exception in "
          "command:\"update a set f = 2;\"\n"}});
}

TEST(UPDATE, TEST_4) {
    clearDB();
    check_requests(
        {{"create table a(a int unique);", 0, ""},
         {"insert into a values (1);", 0, ""},
         {"insert into a values (2);", 0, ""},
         {"update a set a = 2;",
          EXCEPTION2NUMB(exc::ExceptionType::duplicated_unique),
          "~~Exception 804 in table a:\n 2 is not unique is in the column "
          "a.\n~~Exception in command:\"update a set a = 2;\"\n"}});
}

TEST(UPDATE, TEST_5) {
    clearDB();
    check_requests(
        {{"create table a(a int not null);", 0, ""},
         {"insert into a values (1);", 0, ""},
         {"insert into a values (2);", 0, ""},
         {"update a set a = null;",
          EXCEPTION2NUMB(exc::ExceptionType::null_not_null),
          "~~Exception 805 in table a:\n a can't contain null "
          "values.\n~~Exception in command:\"update a set a = null;\"\n"}});
}

TEST(UPDATE, TEST_6) {
    clearDB();
    check_requests(
        {{"create table a(a int primary key);", 0, ""},
         {"insert into a values (1);", 0, ""},
         {"insert into a values (2);", 0, ""},
         {"update a set a = 2;",
          EXCEPTION2NUMB(exc::ExceptionType::duplicated_unique),
          "~~Exception 804 in table a:\n 2 is not unique is in the column "
          "a.\n~~Exception in command:\"update a set a = 2;\"\n"}});
}

TEST(UPDATE, TEST_7) {
    clearDB();
    check_requests(
        {{"create table a(a int primary key);", 0, ""},
         {"insert into a values (1);", 0, ""},
         {"insert into a values (2);", 0, ""},
         {"update a set a = '2';",
          EXCEPTION2NUMB(exc::ExceptionType::set_data_type_mismatch),
          "~~Exception 601:\n value a is not compatible with data type "
          "int.\n~~Exception in command:\"update a set a = '2';\"\n"}});
}
