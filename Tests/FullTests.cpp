#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../Server/Core/Exception.h"
#include "../Server/Our.h"
#include "Test.h"

Client client("localhost", 11234);

TEST(CREATE_TABLE, TEST_1) {
    clearDB();
    check_requests(
        {{"create table a (b int);", 0, ""},
         {"show create table a;", 0, "CREATE TABLE a(\n    b int\n);\n"}},
        client);
}

TEST(CREATE_TABLE, TEST_2) {
    clearDB();
    check_requests(
        {{"create table a (b int)", EXCEPTION2NUMB(exc::ExceptionType::syntax),
          "~~Exception 1:\n wrong syntax!\n~~Exception in command:\"create "
          "table a (b int)\"\n"}},
        client);
}

TEST(CREATE_TABLE, TEST_3) {
    clearDB();
    check_requests({{"create table a(b int not null);", 0, ""}}, client);
}

TEST(CREATE_TABLE, TEST_4) {
    clearDB();
    check_requests(
        {{"create table a(b int not null not null);",
          EXCEPTION2NUMB(exc::ExceptionType::redundant_constraints),
          "~~Exception 802:\n duplicate of constraint b in column a.\n"
          "~~Exception in command:\"create table a(b int not null not "
          "null);\"\n"}},
        client);
}

TEST(CREATE_TABLE, TEST_5) {
    clearDB();
    check_requests({{"create table a(b int not null, c real unique, d "
                     "varchar(100) primary key);",
                     0, ""},
                    {"show create table a;", 0,
                     "CREATE TABLE a(\n"
                     "    b int not null,\n"
                     "    c real unique,\n"
                     "    d varchar(100) primary key\n);\n"}},
                   client);
}

TEST(CREATE_TABLE, TEST_6) {
    clearDB();
    check_requests(
        {{"create table sfs;", EXCEPTION2NUMB(exc::ExceptionType::syntax),
          "~~Exception 1:\n wrong syntax!\n"
          "~~Exception in command:\"create table sfs;\"\n"}},
        client);
}

TEST(CREATE_TABLE, TEST_7) {
    clearDB();
    check_requests(
        {{"create table a(i int);", 0, ""},
         {"create table b(i int);", 0, ""},
         {"create table c(i int);", 0, ""},
         {"show create table a;", 0, "CREATE TABLE a(\n    i int\n);\n"},
         {"show create table b;", 0, "CREATE TABLE b(\n    i int\n);\n"},
         {"show create table c;", 0, "CREATE TABLE c(\n    i int\n);\n"}},
        client);
}

TEST(CREATE_TABLE, TEST_8) {
    clearDB();
    check_requests({{"create table a(MyColumn int, mycolumn varchar(100));",
                     EXCEPTION2NUMB(exc::ExceptionType::repeat_column_in_table),
                     "~~Exception 5:\n repeat column MyColumn in table a.\n"
                     "~~Exception in command:\"create table a(MyColumn int, "
                     "mycolumn varchar(100));\"\n"}},
                   client);
}

TEST(CREATE_TABLE, TEST_9) {
    clearDB();
    check_requests(
        {{"create table a(MyColumn int, q varchar(100));", 0, ""},
         {"create table a(f int);",
          EXCEPTION2NUMB(exc::ExceptionType::create_table_repeat_table_name),
          "~~Exception 1002 in create table a:\n this table name is "
          "repeated!\n~~Exception in command:\"create table a(f int);\"\n"}},
        client);
}

TEST(CREATE_TABLE, TEST_10) {
    clearDB();
    check_requests(
        {{"create table 1a(f int);", EXCEPTION2NUMB(exc::ExceptionType::syntax),
          "~~Exception 1:\n wrong syntax!\n"
          "~~Exception in command:\"create table 1a(f int);\"\n"}},
        client);
}

TEST(CREATE_TABLE, TEST_11) {
    clearDB();
    check_requests(
        {{"create table a(1f int);", EXCEPTION2NUMB(exc::ExceptionType::syntax),
          "~~Exception 1:\n wrong syntax!\n"
          "~~Exception in command:\"create table a(1f int);\"\n"}},
        client);
}

TEST(CREATE_TABLE, TEST_12) {
    clearDB();
    check_requests(
        {{"create table a(b varchar(100) not null unique primary key);", 0, ""},
         {"show create table a;", 0,
          "CREATE TABLE a(\n    b varchar(100) primary key not null "
          "unique\n);\n"}},
        client);
}

TEST(CREATE_TABLE, TEST_13) {
    clearDB();
    check_requests(
        {{"create table a(b int primary key, c int primary key);",
          EXCEPTION2NUMB(exc::ExceptionType::duplicated_primary_key_in_column),
          "~~Exception 803 in table a:\n primary key is used in the column b "
          "and in c.\n"
          "~~Exception in command:\"create table a(b int primary key, c int "
          "primary key);\"\n"}},
        client);
}

TEST(SHOW_CREATE_TABLE, TEST_1) {
    clearDB();
    check_requests({{"show ghgh;", EXCEPTION2NUMB(exc::ExceptionType::syntax),
                     "~~Exception 1:\n wrong syntax!\n"
                     "~~Exception in command:\"show ghgh;\"\n"}},
                   client);
}

TEST(SHOW_CREATE_TABLE, TEST_2) {
    clearDB();
    check_requests(
        {{"show create table e;",
          EXCEPTION2NUMB(exc::ExceptionType::access_table_nonexistent),
          "\n~~Exception 701:\n table e nonexistent.\n~~Exception in "
          "command:\"show create table e;\"\n"}},
        client);
}

TEST(SYNTAX, TEST_1) {
    clearDB();
    check_requests({{"CreAte    \n  TablE   NamE \n ( A ReAl);", 0, ""}},
                   client);
}

TEST(SYNTAX, TEST_2) {
    clearDB();
    check_requests({{"fdgd;", EXCEPTION2NUMB(exc::ExceptionType::syntax),
                     "~~Exception 1:\n wrong syntax!\n~~Exception in "
                     "command:\"fdgd;\"\n"}},
                   client);
}

TEST(SYNTAX, TEST_3) {
    clearDB();
    check_requests({{"CREATE TABLE TAB(F VARCHAR(100));", 0, ""}}, client);
}

TEST(SYNTAX, TEST_4) {
    clearDB();
    check_requests({{"create table a(b varchar(100));", 0, ""},
                    {"insert into a values ('\\' ;');", 0, ""}},
                   client);
}

TEST(DROP_TABLE, TEST_1) {
    clearDB();
    check_requests(
        {{"drop table tr;",
          EXCEPTION2NUMB(exc::ExceptionType::access_table_nonexistent),
          "~~Exception 701:\n table tr nonexistent."
          "\n~~Exception in command:\"drop table tr;\"\n"}},
        client);
}

TEST(DROP_TABLE, TEST_2) {
    clearDB();
    check_requests(
        {{"create table a(b int);", 0, ""},
         {"create table b(c real);", 0, ""},
         {"drop table a;", 0, ""},
         {"create table a(c varchar(100));", 0, ""},
         {"show create table b;", 0, "CREATE TABLE b(\n    c real\n);\n"}},
        client);
}

TEST(DROP_TABLE, TEST_3) {
    clearDB();
    check_requests({{"drop table;", EXCEPTION2NUMB(exc::ExceptionType::syntax),
                     "~~Exception 1:\n wrong syntax!\n"
                     "~~Exception in command:\"drop table;\"\n"}},
                   client);
}

TEST(SELECT, TEST_1) {
    clearDB();
    check_requests({{"create table a(a int, b real, c varchar(100));", 0, ""},
                    {"select * from a;", 0, ""}},
                   client);
}

TEST(SELECT, TEST_2) {
    clearDB();
    check_requests({{"create table a(a int, b int, c int);", 0, ""},
                    {"select *, c, a from a;", 0, ""}},
                   client);
}

TEST(SELECT, TEST_3) {
    clearDB();
    check_requests(
        {{"create table a(a int, b real, c varchar(100));", 0, ""},
         {"select c, * from a;", EXCEPTION2NUMB(exc::ExceptionType::syntax),
          "~~Exception 1:\n wrong syntax!\n"
          "~~Exception in command:\"select c, * from a;\"\n"}},
        client);
}

TEST(SELECT, TEST_4) {
    clearDB();
    check_requests(
        {{"select * from a;",
          EXCEPTION2NUMB(exc::ExceptionType::access_table_nonexistent),
          "~~Exception 701:\n table a nonexistent.\n"
          "~~Exception in command:\"select * from a;\"\n"}},
        client);
}

TEST(SELECT, TEST_5) {
    clearDB();
    check_requests(
        {{"create table a(a int, b real, c varchar(100));", 0, ""},
         {"select *, c, a, f from a;",
          EXCEPTION2NUMB(exc::ExceptionType::access_column_nonexistent),
          "~~Exception 702:\n column f in table a nonexistent.\n"
          "~~Exception in command:\"select *, c, a, f from a;\"\n"}},
        client);
}

TEST(SELECT, TEST_6) {
    clearDB();
    check_requests(
        {{"create table a(a int, b real, c varchar(100));", 0, ""},
         {"drop table a;", 0, ""},
         {"select * from a;",
          EXCEPTION2NUMB(exc::ExceptionType::access_table_nonexistent),
          "~~Exception 701:\n table a nonexistent.\n"
          "~~Exception in command:\"select * from a;\"\n"}},
        client);
}

TEST(SELECT, TEST_7) {
    clearDB();
    check_requests(
        {{"create table a(a int, b real, c varchar(100));", 0, ""},
         {"insert into a values (1, 0, '1');", 0, ""},
         {"insert into a values (1, 1, '0');", 0, ""},
         {"insert into a values (0, 1, '1');", 0, ""},
         {"select * from a where a = 0;", 0, "a: 0\nb: 1.000000\nc: 1\n"}},
        client);
}

TEST(SELECT, TEST_8) {
    clearDB();
    check_requests(
        {{"create table a(a int);", 0, ""},
         {"insert into a values (1);", 0, ""},
         {"select * from a where a = '1';",
          EXCEPTION2NUMB(exc::ExceptionType::compare_data_type_mismatch),
          "~~Exception 605:\n can't compare int and varchar.\n~~Exception in command:\"select * from a where a = '1';\"\n"}},
        client);
}

TEST(INSERT, TEST_1) {
    clearDB();
    check_requests({{"create table a(a int, b real, c varchar(100));", 0, ""},
                    {"insert into a values (3, 2.2, 'Hello');", 0, ""},
                    {"select * from a;", 0, "a: 3\nb: 2.200000\nc: Hello\n"}},
                   client);
}

TEST(INSERT, TEST_2) {
    clearDB();
    check_requests(
        {{"create table a(a int, b real, c varchar(100));", 0, ""},
         {"insert into a (c, a, b) values ('Hello', 3, 2.2);", 0, ""},
         {"select * from a;", 0, "a: 3\nb: 2.200000\nc: Hello\n"}},
        client);
}

TEST(INSERT, TEST_3) {
    clearDB();
    check_requests(
        {{"create table a(a int, b real, c varchar(100));", 0, ""},
         {"insert into a values (2.2, 3.3, 'Hello');",
          EXCEPTION2NUMB(exc::ExceptionType::set_data_type_mismatch),
          "~~Exception 601:\n value a is not compatible with data type int.\n"
          "~~Exception in command:\"insert into a values (2.2, 3.3, "
          "'Hello');\"\n"}},
        client);
}

TEST(INSERT, TEST_4) {
    clearDB();
    check_requests({{"create table a(a int, b real, c varchar(100));", 0, ""},
                    {"insert into a values (-2, -1, '');", 0, ""},
                    {"select * from a;", 0, "a: -2\nb: -1.000000\nc: \n"}},
                   client);
}

TEST(INSERT, TEST_5) {
    clearDB();
    check_requests({{"create table a(a int, b real, c varchar(100));", 0, ""},
                    {"insert into a values (2, 3.3.3, 'Hello');",
                     EXCEPTION2NUMB(exc::ExceptionType::syntax),
                     "~~Exception 1:\n wrong syntax!\n"
                     "~~Exception in command:\"insert into a values (2, 3.3.3, "
                     "'Hello');\"\n"}},
                   client);
}

TEST(INSERT, TEST_6) {
    clearDB();
    check_requests(
        {{"create table a(a int, b real, c varchar(100));", 0, ""},
         {"insert into a values ('Hello', 3.3, 'Hello');",
          EXCEPTION2NUMB(exc::ExceptionType::set_data_type_mismatch),
          "~~Exception 601:\n value a is not compatible with data type int.\n"
          "~~Exception in command:\"insert into a values ('Hello', 3.3, "
          "'Hello');\"\n"}},
        client);
}

TEST(INSERT, TEST_7) {
    clearDB();
    check_requests({{"create table a(a int, b real, c varchar(100));", 0, ""},
                    {"insert into a values (2, 3.3, 4);",
                     EXCEPTION2NUMB(exc::ExceptionType::set_data_type_mismatch),
                     "~~Exception 601:\n value c is not compatible with data "
                     "type varchar.\n~~Exception in command:\"insert into a "
                     "values (2, 3.3, 4);\"\n"}},
                   client);
}

TEST(INSERT, TEST_8) {
    clearDB();
    check_requests(
        {{"create table a(a int, b real, c varchar(100));", 0, ""},
         {"insert into a(b, a, c) values (2, 3.3, 'Hello');",
          EXCEPTION2NUMB(exc::ExceptionType::set_data_type_mismatch),
          "~~Exception 601:\n value a is not compatible with data type "
          "int.\n~~Exception in command:\"insert into a(b, a, c) values (2, "
          "3.3, "
          "'Hello');\"\n"}},
        client);
}

TEST(INSERT, TEST_9) {
    clearDB();
    check_requests(
        {{"insert into a values (2, 3.3, 'Hello');",
          EXCEPTION2NUMB(exc::ExceptionType::access_table_nonexistent),
          "~~Exception 701:\n table a nonexistent.\n~~Exception in "
          "command:\"insert into a values (2, 3.3, 'Hello');\"\n"}},
        client);
}

TEST(INSERT, TEST_10) {
    clearDB();
    check_requests(
        {{"create table a(a int, b real, c varchar(100));", 0, ""},
         {"insert into a(a, b, f) values (2, 3.3, 'Hello');",
          EXCEPTION2NUMB(exc::ExceptionType::access_column_nonexistent),
          "~~Exception 702:\n column f in table a nonexistent.\n"
          "~~Exception in command:\"insert into a(a, b, f) values (2, 3.3, "
          "'Hello');\"\n"}},
        client);
}

TEST(INSERT, TEST_11) {
    clearDB();
    check_requests({{"create table a(a int, b real, c varchar(100));", 0, ""},
                    {"insert into a(a) values (-2);", 0, ""},
                    {"select * from a;", 0, "a: -2\nb: null\nc: \n"}},
                   client);
}

TEST(INSERT, TEST_12) {
    clearDB();
    check_requests({{"create table a(a int, b real, c varchar(100));", 0, ""},
                    {"insert into a(a) values (8);", 0, ""},
                    {"select * from a;", 0, "a: 8\nb: null\nc: \n"}},
                   client);
}

TEST(INSERT, TEST_13) {
    clearDB();
    check_requests(
        {{"create table a(a int not null, b real primary key, c varchar(100) "
          "unique);",
          0, ""},
         {"insert into a values (-2, 0.1, 'Hello world!');", 0, ""},
         {"select * from a;", 0, "a: -2\nb: 0.100000\nc: Hello world!\n"}},
        client);
}

TEST(INSERT, TEST_14) {
    clearDB();
    check_requests(
        {{"create table a(a int not null, b real primary key, c varchar(100) "
          "unique);",
          0, ""},
         {"insert into a(b, c) values (0.1, 'H M!');",
          EXCEPTION2NUMB(exc::ExceptionType::null_not_null),
          "~~Exception 805 in table a:\n a can't contain null values.\n"
          "~~Exception in command:\"insert into a(b, c) values (0.1, 'H "
          "M!');\"\n"}},
        client);
}

TEST(INSERT, TEST_15) {
    clearDB();
    check_requests(
        {{"create table a(a int not null, b real primary key, c varchar(100) "
          "unique);",
          0, ""},
         {"insert into a values (1, 0, 'H M!');", 0, ""},
         {"insert into a values (12, 0, 'H!');",
          EXCEPTION2NUMB(exc::ExceptionType::duplicated_unique),
          "~~Exception 804 in table a:\n 0.000000 is not unique is in the column "
          "b.\n~~Exception in command:\"insert into a values (12, 0, "
          "'H!');\"\n"}},
        client);
}

TEST(INSERT, TEST_16) {
    clearDB();
    check_requests({{"create table a(a int not null, b real primary key, c "
                     "varchar(100) unique);",
                     0, ""},
                    {"insert into a values (1, 0, 'H!');", 0, ""},
                    {"insert into a values (12, 1, 'H!');",
                     EXCEPTION2NUMB(exc::ExceptionType::duplicated_unique),
                     "~~Exception 804 in table a:\n null is not unique is in "
                     "the column c.\n~~Exception in command:\"insert into a "
                     "values (12, 1, 'H!');\"\n"}},
                   client);
}

TEST(INSERT, TEST_17) {
    clearDB();
    check_requests(
        {{"create table a(a int not null, b real primary key, c varchar(100) "
          "unique);",
          0, ""},
         {"insert into a values (1, 0, null);", 0, ""},
         {"insert into a(a, b) values (12, 1);",
          EXCEPTION2NUMB(exc::ExceptionType::duplicated_unique),
          "~~Exception 804 in table a:\n null is not unique is in the column "
          "c.\n~~Exception in command:\"insert into a(a, b) values (12, "
          "1);\"\n"}},
        client);
}

TEST(INSERT, TEST_18) {
    clearDB();
    check_requests({{"create table a(a int not null, b real primary key, c "
                     "varchar(100) unique);",
                     0, ""},
                    {"insert into a values (1, 0, 'H!');", 0, ""},
                    {"insert into a(a, b) values (12, 1);"
                     "select * from a;",
                     0, "a: 1\nb: 0.000000\nc: H!\na: 12\nb: 1.000000\nc: \n"}},
                   client);
}

TEST(INSERT, TEST_19) {
    clearDB();
    check_requests({{"create table a(a int, b real, c varchar(100));", 0, ""},
                    {"insert into a(a, a) values (12, 1);",
                     EXCEPTION2NUMB(exc::ExceptionType::repeat_column),
                     "~~Exception 6:\n repeat column a.\n~~Exception in "
                     "command:\"insert "
                     "into a(a, a) values (12, 1);\"\n"}},
                   client);
}

TEST(INSERT, TEST_20) {
    clearDB();
    check_requests(
        {{"create table a(a int, b real, c varchar(100));", 0, ""},
         {"insert into a(a) values (12, 1);",
          EXCEPTION2NUMB(exc::ExceptionType::insert_constants_more_columns),
          "~~Exception 1101:\n the number of constants is more than "
          "columns.\n"
          "~~Exception in command:\"insert into a(a) values (12, 1);\"\n"}},
        client);
}

TEST(INSERT, TEST_21) {
    clearDB();
    check_requests(
        {{"create table a(a int, b real, c varchar(100));", 0, ""},
         {"insert into a values (12, 12, ' ') where b = 2 + 1;",
          EXCEPTION2NUMB(exc::ExceptionType::syntax),
          "~~Exception 1:\n wrong syntax!\n"
          "~~Exception in command:\"insert into a values (12, 12, ' ') where "
          "b = "
          "2 + 1;\"\n"}},
        client);
}

// TEST(DELETE, TEST_1) {
//    clearDB();
//    check_requests(
//        {{"create table a(a int, b real, c varchar(100));", 0, ""},
//         {"insert into a values (1, 0, '1');", 0, ""},
//         {"insert into a values (1, 1, '0');", 0, ""},
//         {"insert into a values (0, 1, '1');", 0, ""},
//         {"delete from a where a = 0;", 0, ""},
//         {"select * from a;", 0, "a: 1\nb: 0\nc: '1'\na: 1\nb: 1\nc: '0'\n"},
//         {"delete from a where b = 0;", 0, ""},
//         {"select * from a;", 0, "a: 1\nb: 1\nc: '0'\n"},
//         {"delete from a where c = '0';", 0, ""},
//         {"select * from a;", 0, ""}},
//        client);
//}
//
// TEST(DELETE, TEST_2) {
//    clearDB();
//    check_requests({{"create table a(a int, b real, c varchar(100));", 0, ""},
//                    {"delete from a where c = '0';", 0, ""},
//                    {"select * from a;", 0, ""}},
//                   client);
//}
//
// TEST(DELETE, TEST_3) {
//    clearDB();
//    check_requests(
//        {{"delete from a where c = '0';",
//          EXCEPTION2NUMB(exc::ExceptionType::access_table_nonexistent),
//          "~~Exception 701:\n table a nonexistent.\n~~Exception in "
//          "command:\"delete from a where c = '0';\"\n"}},
//        client);
//}
//
// TEST(DELETE, TEST_4) {
//    clearDB();
//    check_requests(
//        {{"create table a(a int, b real, c varchar(100));", 0, ""},
//         {"delete from a where f = '0';",
//          EXCEPTION2NUMB(exc::ExceptionType::access_column_nonexistent),
//          "~~Exception 702:\n column f in table a nonexistent.\n~~Exception "
//          "in "
//          "command:\"delete from a where f = '0';\"\n"}},
//        client);
//}
//
// TEST(DELETE, TEST_5) {
//    clearDB();
//    check_requests(
//        {{"create table a(a int, b real, c varchar(100));", 0, ""},
//         {"delete from a where b = '0';",
//          EXCEPTION2NUMB(exc::ExceptionType::compare_data_type_mismatch),
//          "~~Exception 602:\n can't compare real and
//          varchar(100).\n~~Exception in " "command:\"delete from a where b =
//          '0';\"\n"}},
//        client);
//}
//
// TEST(DELETE, TEST_6) {
//    clearDB();
//    check_requests(
//        {{"create table a(a int, b real, c varchar(100));", 0, ""},
//         {"insert into a values (1, 0, '1');", 0, ""},
//         {"insert into a values (1, 1, '0');", 0, ""},
//         {"insert into a values (0, 1, '1');", 0, ""},
//         {"insert into a values (0, 0, '1');", 0, ""},
//         {"delete from a where b = a or a = 1;", 0, ""},
//         {"select a from a;", 0, "a: 0\n"}},
//        client);
//}

TEST(UPDATE, TEST_1) {
    clearDB();
    check_requests({{"create table a(a int, b real, c varchar(100));", 0, ""},
                    {"insert into a values (1, 0, '1');", 0, ""},
                    {"insert into a values (1, 1, '0');", 0, ""},
                    {"insert into a values (0, 1, '1');", 0, ""},
                    {"update a set a = 2;", 0, ""},
                    {"select * from a;", 0,
                     "a: 2\nb: 1.000000\nc: 1\na: 2\nb: 1.000000\nc: 1\na: "
                     "2\nb: 1.000000\nc: 1\n"},
                    {"update a set b = 3.45, c = 'H';", 0, ""},
                    {"select * from a;", 0,
                     "a: 2\nb: 3.450000\nc: H\na: 2\nb: 3.450000\nc: H\na: "
                     "2\nb: 3.450000\nc: H\n"}},
                   client);
}

TEST(UPDATE, TEST_2) {
    clearDB();
    check_requests(
        {{"create table a(a int, b real, c varchar(100));", 0, ""},
         {"insert into a values (1, 0, '1');", 0, ""},
         {"insert into a values (1, 1, '0');", 0, ""},
         {"insert into a values (0, 1, '1');", 0, ""},
         {"update a set f = 2;",
          EXCEPTION2NUMB(exc::ExceptionType::access_column_nonexistent),
          "~~Exception 702:\n column f in table a nonexistent.\n~~Exception "
          "in "
          "command:\"update a set f = 2;\"\n"}},
        client);
}

TEST(UPDATE, TEST_3) {
    clearDB();
    check_requests(
        {{"update a set f = 2;",
          EXCEPTION2NUMB(exc::ExceptionType::access_table_nonexistent),
          "~~Exception 701:\n table a nonexistent.\n~~Exception in "
          "command:\"update a set f = 2;\"\n"}},
        client);
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
          "a.\n~~Exception in command:\"update a set a = 2;\"\n"}},
        client);
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
          "values.\n~~Exception in command:\"update a set a = null;\"\n"}},
        client);
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
          "a.\n~~Exception in command:\"update a set a = 2;\"\n"}},
        client);
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
          "int.\n~~Exception in command:\"update a set a = '2';\"\n"}},
        client);
}

TEST(WHERE, TEST_1) {
    clearDB();
    check_requests(
        {{"create table a (a varchar(100), b varchar(100));", 0, ""},
         {"insert into a values('ab', 'ab');", 0, ""},
         {"select * from a where '1';", 0, ""},
         {"insert into a values('ab', 'abc');", 0, ""},
         {"insert into a values('ab', 'b');", 0, ""},
         {"insert into a values('ab', 'bb');", 0, ""},
         {"insert into a values('ab', 'a');", 0, ""},
         {"insert into a values('ab', 'aa');", 0, ""},
         {"insert into a values('ab', '');", 0, ""},
         {"select * from a where a < b;", 0,
          "a: ab\nb: abc\na: ab\nb: b\na: ab\nb: bb\n"},
         {"select * from a where a <= b;", 0,
          "a: ab\nb: ab\na: ab\nb: abc\na: ab\nb: b\na: ab\nb: bb\n"},
         {"select * from a where a = b;", 0, "a: ab\nb: ab\n"},
         {"select * from a where a >= b;", 0,
          "a: ab\nb: ab\na: ab\nb: a\na: ab\nb: aa\na: ab\nb: \n"},
         {"select * from a where a > b;", 0,
          "a: ab\nb: a\na: ab\nb: aa\na: ab\nb: \n"},
         {"select * from a where a >= b and b > 'aa' or b = 'b';", 0,
          "a: ab\nb: ab\na: ab\nb: b\n"},
         {"select * from a where a < b and b != abc;", 0,
          "a: ab\nb: b\na: ab\nb: bb\n"},
         {"select * from a where a + 'b' = 'abb';", -1, ""},  // TODO
         {"select * from a where a - 'b' = 'a';", -1, ""},    // TODO
         {"select * from a where a * 2 = 'abb';", -1, ""},    // TODO
         {"select * from a where a / 'b' = 'abb';", -1, ""}},
        client);  // TODO
}

TEST(WHERE, TEST_2) {
    clearDB();
    check_requests(
        {{"create table a (a int, b int);", 0, ""},
         {"insert into a values(-3, 3);", 0, ""},
         {"select * from a where 1;", 0, "a: -3\nb: 3"},
         {"select * from a where 0;", 0, ""},
         {"insert into a values(-1, -1);", 0, ""},
         {"insert into a values(-1, -10);", 0, ""},
         {"insert into a values(-1, 10);", 0, ""},
         {"select * from a where a < b;", 0, "a: -3\nb: 3\na: -1\nb: 10\n"},
         {"select * from a where a <= b;", 0,
          "a: -3\nb: 3\na: -1\nb: -1\na: -1\nb: 10\n"},
         {"select * from a where (12 + (4.5 * 2 / 3) + 2.0 * 2 - a * (-1)) / 2 "
          "= b + a;",
          0, "a: -1\nb: 10\n"},
         {"select * from a where a >= -0.567;", 0,
          "a: -3\nb: 3\na: -1\nb: -1\na: -1\nb: 10\n"},
         {"select * from a where b != a and b != 10 and b != -10;", 0,
          "a: -3\nb: 3\n"}},
        client);
}

TEST(WHERE, TEST_3) {
    clearDB();
    check_requests(
        {{"create table a (a real, b real);", 0, ""},
         {"insert into a values(-3.25, 3);", 0, ""},
         {"select * from a where 0.0;", 0, ""},
         {"select * from a where 0.12;", 0, "a: -3.25\nb: 3\n"},
         {"insert into a values(-1.0, -1);", 0, ""},
         {"insert into a values(0.123, -10);", 0, ""},
         {"select * from a where a + b < (-1.234 * (-1) + 2.05 * 3 - 1.2 / "
          "2.1) / (-3.7);",
          0, "a: -1.0\nb: -1.0\na: 0.123\nb: -10.0\n"},
         {"select * from a where a  <= b;", 0,
          "a: -3.25\nb: 3.0\na: -1.0\nb: -1.0\n"},
         {"select * from a where a / 3.25 * 3 = b * -1;", 0,
          "a: -3.25\nb: 3.0\n"},
         {"select * from a where a >= b;", 0,
          "a: -1.0\nb: -1.0\na: 0.123\nb: -10.0\n"},
         {"select * from a where a > 0;", 0, "a: 0.123\nb: -10.0\n"}},
        client);
}

TEST(WHERE, TEST_4) {
    clearDB();
    check_requests(
        {{"create table a (a int, b real, c varchar(100));", 0, ""},
         {"insert into a values(1, 2.35, '67 89');", 0, ""},
         {"select * from a where a = b - 1.35;", 0,
          "a: 1\nb: 2.35\nc: 67 89\n"},
         {"select * from a where a <= c;", -1, ""},  // TODO
         {"select * from a where b = c;", -1, ""},   // TODO
         {"select * from a where -1;", -1, ""},      // TODO
         {"select * from a where not not a = a;", 0,
          "a: 1\nb: 2.35\nc: 67 89\n"},
         {"select * from a where not a != a;", 0, "a: 1\nb: 2.35\nc: 67 89\n"},
         {"select * from a where (a = 1) < 3;", 0, "a: 1\nb: 2.35\nc: 67 89\n"},
         {"select * from a where (a = 1) > (3 != 3);", 0,
          "a: 1\nb: 2.35\nc: 67 89\n"},
         {"select * from a where (a = 1) <= (3 != 3);", 0, ""},
         {"select * from a where (a = 1) * (3 != 3);", 0,
          "a: 1\nb: 2.35\nc: 67 89\n"},
         {"select * from a where (a = 1) + (3 != 3);", 0,
          "a: 1\nb: 2.35\nc: 67 89\n"},
         {"select * from a where (a = 1) / (3 != 3);", 0,
          "a: 1\nb: 2.35\nc: 67 89\n"},
         {"select * from a where (a = 1) - (3 != 3);", 0, ""},
         {"insert into a values(3, 0, '');", 0, ""},
         {"select * from a where c =;", -1, ""},  // TODO
         {"select * from a where not a < 1 or b = 2 and c = '';", 0,
          "a: 3\nb: 0\nc: \n"},  // Пока работает так. Разработчик Виктор обещал
                                 // пофиксить к паре, но не сказал к какой. Ну
                                 // или это особенность субд
         {"select * from a where a >= 1 and (b = 2 or c = '');", 0,
          "a: 3\nb: 0\nc: \n"},
         {"select * from a where a - 3 = b = 0;", 0,
          "a: 1\nb: 2.35\nc: 67 89\n"},
         {"select * from a where (a - 3 = b) * 3 = 0;", -1, ""},  // TODO
         {"select * from a where a - 3 = b * 1 = 0;", 0, "a: 3\nb: 0\nc: \n"}},
        client);
}
