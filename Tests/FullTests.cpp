#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../App/Core/Exception.h"
#include "../App/Our.h"
#include "Test.h"

TEST(CREATE_TABLE, TEST_1) {
    clearDB();
    std::vector<std::string> requests = {"create table a (b int);",
                                         "show create table a;"};
    std::vector<unsigned int> exceptions = {0, 0};
    std::vector<std::string> answers = {"", "CREATE TABLE a(\n    b int\n);\n"};
    for (unsigned int i = 0; i < requests.size(); i++) {
        CHECK_REQUEST(requests[i], exceptions[i], answers[i]);
    }
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
        "create table a(b int not null, c real unique, d varchar(100) primary "
        "key);"
        "show create table a;",
        0,
        "CREATE TABLE a(\n"
        "    b int not null,\n"
        "    c real unique,\n"
        "    d varchar(100) primary key\n);\n");
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
    CHECK_REQUEST("create table a(MyColumn int, mycolumn varchar(100));",
                  exc::ExceptionType::repeat_column_in_table,
                  "~~Exception 5:\n repeat column MyColumn in table a.\n"
                  "~~Exception in command:\"create table a(MyColumn int, "
                  "mycolumn varchar(100));\"\n");
}

TEST(CREATE_TABLE, TEST_9) {
    clearDB();
    CHECK_REQUEST(
        "create table a(MyColumn int, q varchar(100)); create table a(f int);",
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
        "create table a(b varchar(100) not null unique primary key);"
        "show create table a;",
        0,
        "CREATE TABLE a(\n    b varchar(100) primary key not null "
        "unique\n);\n");
}

TEST(CREATE_TABLE, TEST_13) {
    clearDB();
    CHECK_REQUEST("create table a(b int primary key, c int primary key);",
                  exc::ExceptionType::duplicated_primary_key_in_column,
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
                  "\n~~Exception 701:\n table e nonexistent.\n~~Exception in "
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
    CHECK_REQUEST("CREATE TABLE TAB(F varchar(100));", 0, "");
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
        "table a(c varchar(100)); show create table b;",
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
        "create table a(a int, b real, c varchar(100));"
        "select * from a;",
        0, "");
}

TEST(SELECT, TEST_2) {
    clearDB();
    CHECK_REQUEST(
        "create table a(a int, b int, c int);"
        "select *, c, a from a;",
        0, "");
}

TEST(SELECT, TEST_3) {
    clearDB();
    CHECK_REQUEST(
        "create table a(a int, b real, c varchar(100));"
        "select c, * from a;",
        exc::ExceptionType::syntax,
        "~~Exception 1:\n wrong syntax!\n"
        "~~Exception in command:\"select c, * from a;\"\n");
}

TEST(SELECT, TEST_4) {
    clearDB();
    CHECK_REQUEST("select * from a;",
                  exc::ExceptionType::access_table_nonexistent,
                  "~~Exception 701:\n table a nonexistent.\n"
                  "~~Exception in command:\"select * from a;\"\n");
}

TEST(SELECT, TEST_5) {
    clearDB();
    CHECK_REQUEST(
        "create table a(a int, b real, c varchar(100));"
        "select *, c, a, f from a;",
        exc::ExceptionType::access_column_nonexistent,
        "~~Exception 702:\n column f in table a nonexistent.\n"
        "~~Exception in command:\"select *, c, a, f from a;\"\n");
}

TEST(SELECT, TEST_6) {
    clearDB();
    CHECK_REQUEST(
        "create table a(a int, b real, c varchar(100));"
        "drop table a;"
        "select * from a;",
        exc::ExceptionType::access_table_nonexistent,
        "~~Exception 701:\n table a nonexistent.\n"
        "~~Exception in command:\"select * from a;\"\n");
}

TEST(SELECT, TEST_7) {
    clearDB();
    CHECK_REQUEST(
        "create table a(a int, b real, c varchar(100));"
        "insert into a values (1, 0, '1');"
        "insert into a values (1, 1, '0');"
        "insert into a values (0, 1, '1');"
        "select * from a where a = 0;",
        0, "a: 0\nb: 1.000000\nc: 1\n");
}

TEST(SELECT, TEST_8) {
    clearDB();
    CHECK_REQUEST(
        "create table a(a int);"
        "insert into a values (1);"
        "select * from a where a = '1';",
        exc::ExceptionType::compare_data_type_mismatch,
        "~~Exception 605:\n can't compare int and varchar.\n"
        "~~Exception in command:\"select * from a where a = '1';\"\n");
}

TEST(INSERT, TEST_1) {
    clearDB();
    CHECK_REQUEST(
        "create table a(a int, b real, c varchar(100));"
        "insert into a values (3, 2.2, 'Hello');"
        "select * from a;",
        0, "a: 3\nb: 2.200000\nc: Hello\n");
}

TEST(INSERT, TEST_2) {
    clearDB();
    CHECK_REQUEST(
        "create table a(a int, b real, c varchar(100));"
        "insert into a (c, a, b) values ('Hello', 3, 2.2);"
        "select * from a;",
        0, "a: 3\nb: 2.200000\nc: Hello\n");
}

TEST(INSERT, TEST_3) {
    clearDB();
    CHECK_REQUEST(
        "create table a(a int, b real, c varchar(100));"
        "insert into a values (2.2, 3.3, 'Hello');",
        exc::ExceptionType::set_data_type_mismatch,
        "~~Exception 601:\n value a is not compatible with data type int.\n"
        "~~Exception in command:\"insert into a values (2.2, 3.3, "
        "'Hello');\"\n");
}

TEST(INSERT, TEST_4) {
    clearDB();
    CHECK_REQUEST(
        "create table a(a int, b real, c varchar(100));"
        "insert into a values (-2, -1, '');"
        "select * from a;",
        0, "a: -2\nb: -1.000000\nc: \n");
}

TEST(INSERT, TEST_5) {
    clearDB();
    CHECK_REQUEST(
        "create table a(a int, b real, c varchar(100));"
        "insert into a values (2, 3.3.3, 'Hello');",
        exc::ExceptionType::syntax,
        "~~Exception 1:\n wrong syntax!\n"
        "~~Exception in command:\"insert into a values (2, 3.3.3, "
        "'Hello');\"\n");
}

TEST(INSERT, TEST_6) {
    clearDB();
    CHECK_REQUEST(
        "create table a(a int, b real, c varchar(100));"
        "insert into a values ('Hello', 3.3, 'Hello');",
        exc::ExceptionType::set_data_type_mismatch,
        "~~Exception 601:\n value a is not compatible with data type int.\n"
        "~~Exception in command:\"insert into a values ('Hello', 3.3, "
        "'Hello');\"\n");
}

TEST(INSERT, TEST_7) {
    clearDB();
    CHECK_REQUEST(
        "create table a(a int, b real, c varchar(100));"
        "insert into a values (2, 3.3, 4);",
        exc::ExceptionType::set_data_type_mismatch,
        "~~Exception 601:\n value c is not compatible with data type varchar.\n"
        "~~Exception in command:\"insert into a values (2, 3.3, 4);\"\n");
}

TEST(INSERT, TEST_8) {
    clearDB();
    CHECK_REQUEST(
        "create table a(a int, b real, c varchar(100));"
        "insert into a(b, a, c) values (2, 3.3, 'Hello');",
        exc::ExceptionType::set_data_type_mismatch,
        "~~Exception 601:\n value a is not compatible with data type "
        "int.\n~~Exception in command:\"insert into a(b, a, c) values (2, 3.3, "
        "'Hello');\"\n");
}

TEST(INSERT, TEST_9) {
    clearDB();
    CHECK_REQUEST("insert into a values (2, 3.3, 'Hello');",
                  exc::ExceptionType::access_table_nonexistent,
                  "~~Exception 701:\n table a nonexistent.\n~~Exception in "
                  "command:\"insert into a values (2, 3.3, 'Hello');\"\n");
}

TEST(INSERT, TEST_10) {
    clearDB();
    CHECK_REQUEST(
        "create table a(a int, b real, c varchar(100));"
        "insert into a(a, b, f) values (2, 3.3, 'Hello');",
        exc::ExceptionType::access_column_nonexistent,
        "~~Exception 702:\n column f in table a nonexistent.\n"
        "~~Exception in command:\"insert into a(a, b, f) values (2, 3.3, "
        "'Hello');\"\n");
}

TEST(INSERT, TEST_11) {
    clearDB();
    CHECK_REQUEST(
        "create table a(a int, b real, c varchar(100));"
        "insert into a(a) values (-2);"
        "select * from a;",
        0, "a: -2\nb: null\nc: \n");
}

TEST(INSERT, TEST_12) {
    clearDB();
    CHECK_REQUEST(
        "create table a(a int, b real, c varchar(100));"
        "insert into a(a) values (8);"
        "select * from a;",
        0, "a: 8\nb: null\nc: \n");
}

TEST(INSERT, TEST_13) {
    clearDB();
    CHECK_REQUEST(
        "create table a(a int not null, b real primary key, c varchar(100) "
        "unique);"
        "insert into a values (-2, 0.1, 'Hello world!');"
        "select * from a;",
        0, "a: -2\nb: 0.100000\nc: Hello world!\n");
}

TEST(INSERT, TEST_14) {
    clearDB();
    CHECK_REQUEST(
        "create table a(a int not null, b real primary key, c varchar(100) "
        "unique);"
        "insert into a(b, c) values (0.1, 'H M!');",
        exc::ExceptionType::null_not_null,
        "~~Exception 805 in table a:\n a can't contain null values.\n"
        "~~Exception in command:\"insert into a(b, c) values (0.1, 'H "
        "M!');\"\n");
}

TEST(INSERT, TEST_15) {
    clearDB();
    CHECK_REQUEST(
        "create table a(a int not null, b real primary key, c varchar(100) "
        "unique);"
        "insert into a(a, c, b) values (1, 'H M!', 1);",
        0, "");
}

TEST(INSERT, TEST_16) {
    clearDB();
    CHECK_REQUEST(
        "create table a(a int not null, b real primary key, c varchar(100) "
        "unique);"
        "insert into a values (1, 0, 'H M!');"
        "insert into a values (12, 0, 'H!');",
        exc::ExceptionType::duplicated_unique,
        "~~Exception 804 in table a:\n 0.000000 is not unique is in the column "
        "b.\n~~Exception in command:\"insert into a values (12, 0, 'H!');\"\n");
}

TEST(INSERT, TEST_17) {
    clearDB();
    CHECK_REQUEST(
        "create table a(a int not null, b real primary key, c varchar(100) "
        "unique);"
        "insert into a values (1, 0, 'H!');"
        "insert into a values (12, 1, 'H!');",
        exc::ExceptionType::duplicated_unique,
        "~~Exception 804 in table a:\n null is not unique is in the column "
        "c.\n~~Exception in command:\"insert into a values (12, 1, 'H!');\"\n");
}

TEST(INSERT, TEST_18) {
    clearDB();
    CHECK_REQUEST(
        "create table a(a int not null, b real primary key, c varchar(100) "
        "unique);"
        "insert into a values (1, 0, null);"
        "insert into a(a, b) values (12, 1);",
        exc::ExceptionType::duplicated_unique,
        "~~Exception 804 in table a:\n null is not unique is in the column "
        "c.\n~~Exception in command:\"insert into a(a, b) values (12, 1);\"\n");
}

TEST(INSERT, TEST_19) {
    clearDB();
    CHECK_REQUEST(
        "create table a(a int not null, b real primary key, c varchar(100) "
        "unique);"
        "insert into a values (1, 0, 'H!');"
        "insert into a(a, b) values (12, 1);"
        "select * from a;",
        0, "a: 1\nb: 0.000000\nc: H!\na: 12\nb: 1.000000\nc: \n");
}

TEST(INSERT, TEST_20) {
    clearDB();
    CHECK_REQUEST(
        "create table a(a int, b real, c varchar(100));"
        "insert into a(a, a) values (12, 1);",
        exc::ExceptionType::repeat_column,
        "~~Exception 6:\n repeat column a.\n~~Exception in command:\"insert "
        "into a(a, a) values (12, 1);\"\n");
}

TEST(INSERT, TEST_21) {
    clearDB();
    CHECK_REQUEST(
        "create table a(a int, b real, c varchar(100));"
        "insert into a(a) values (12, 1);",
        exc::ExceptionType::insert_constants_more_columns,
        "~~Exception 1101:\n the number of constants is more than columns.\n"
        "~~Exception in command:\"insert into a(a) values (12, 1);\"\n");
}

TEST(INSERT, TEST_22) {
    clearDB();
    CHECK_REQUEST(
        "create table a(a int, b real, c varchar(100));"
        "insert into a values (12, 12, ' ') where b = 3;",
        exc::ExceptionType::syntax,
        "~~Exception 1:\n wrong syntax!\n"
        "~~Exception in command:\"insert into a values (12, 12, ' ') where b = "
        "3;\"\n");
}

TEST(DELETE, TEST_1) {
    clearDB();
    CHECK_REQUEST(
        "create table a(a int, b real, c varchar(100));"
        "insert into a values (1, 0, '1');"
        "insert into a values (1, 1, '0');"
        "insert into a values (0, 1, '1');"
        "delete from a where a = 0;"
        "select * from a;"
        "delete from a where b = 0;"
        "select * from a;"
        "delete from a where c = '0';"
        "select * from a;",
        0,
        "a: 1\nb: 0.000000\nc: 1\na: 1\nb: 1.000000\nc: 0\na: 1\nb: "
        "0.000000\nc: 1\na: 1\nb: 1.000000\nc: 0\na: 1\nb: 0.000000\nc: 1\n");
}

TEST(DELETE, TEST_2) {
    clearDB();
    CHECK_REQUEST(
        "create table a(a int, b real, c varchar(100));"
        "delete from a where c = '0';"
        "select * from a;",
        0, "");
}

TEST(DELETE, TEST_3) {
    clearDB();
    CHECK_REQUEST("delete from a where c = '0';",
                  exc::ExceptionType::access_table_nonexistent,
                  "~~Exception 701:\n table a nonexistent.\n~~Exception in "
                  "command:\"delete from a where c = '0';\"\n");
}

TEST(DELETE, TEST_4) {
    clearDB();
    CHECK_REQUEST(
        "create table a(a int, b real, c varchar(100));"
        "delete from a where f = '0';",
        0, "");
}

TEST(UPDATE, TEST_1) {
    clearDB();
    CHECK_REQUEST(
        "create table a(a int, b real, c varchar(100));"
        "insert into a values (1, 0, '1');"
        "insert into a values (1, 1, '0');"
        "insert into a values (0, 1, '1');"
        "update a set a = 2;"
        "select * from a;"
        "update a set b = 3.45, c = 'H';"
        "select * from a;",
        0,
        "a: 2\nb: 1.000000\nc: 1\na: 2\nb: 1.000000\nc: 1\na: 2\nb: "
        "1.000000\nc: 1\na: 2\nb: 3.450000\nc: H\na: 2\nb: 3.450000\nc: H\na: "
        "2\nb: 3.450000\nc: H\n");
}

TEST(UPDATE, TEST_2) {
    clearDB();
    CHECK_REQUEST(
        "create table a(a int, b real, c varchar(100));"
        "insert into a values (1, 0, '1');"
        "insert into a values (1, 1, '0');"
        "insert into a values (0, 1, '1');"
        "update a set f = 2;",
        exc::ExceptionType::access_column_nonexistent,
        "~~Exception 702:\n column f in table a nonexistent.\n~~Exception in "
        "command:\"update a set f = 2;\"\n");
}

TEST(UPDATE, TEST_3) {
    clearDB();
    CHECK_REQUEST("update a set f = 2;",
                  exc::ExceptionType::access_table_nonexistent,
                  "~~Exception 701:\n table a nonexistent.\n~~Exception in "
                  "command:\"update a set f = 2;\"\n");
}

TEST(UPDATE, TEST_4) {
    clearDB();
    CHECK_REQUEST(
        "create table a(a int unique);"
        "insert into a values (1);"
        "insert into a values (2);"
        "update a set a = 2;",
        exc::ExceptionType::duplicated_unique,
        "~~Exception 804 in table a:\n 2 is not unique is in the column "
        "a.\n~~Exception in command:\"update a set a = 2;\"\n");
}

TEST(UPDATE, TEST_5) {
    clearDB();
    CHECK_REQUEST(
        "create table a(a int not null);"
        "insert into a values (1);"
        "insert into a values (2);"
        "update a set a = null;",
        exc::ExceptionType::null_not_null,
        "~~Exception 805 in table a:\n a can't contain null "
        "values.\n~~Exception in command:\"update a set a = null;\"\n");
}

TEST(UPDATE, TEST_6) {
    clearDB();
    CHECK_REQUEST(
        "create table a(a int primary key);"
        "insert into a values (1);"
        "insert into a values (2);"
        "update a set a = 2;",
        exc::ExceptionType::duplicated_unique,
        "~~Exception 804 in table a:\n 2 is not unique is in the column "
        "a.\n~~Exception in command:\"update a set a = 2;\"\n");
}

TEST(UPDATE, TEST_7) {
    clearDB();
    CHECK_REQUEST(
        "create table a(a int primary key);"
        "insert into a values (1);"
        "insert into a values (2);"
        "update a set a = '2';",
        exc::ExceptionType::set_data_type_mismatch,
        "~~Exception 601:\n value a is not compatible with data type "
        "int.\n~~Exception in command:\"update a set a = '2';\"\n");
}
