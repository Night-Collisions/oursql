#define CREATE_SERVER

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../Server/Core/Exception.h"
#include "../Server/Our.h"
#include "Test.h"

class REQUEST_TESTS : public ::testing::Test {
   public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void TearDown() override { clearDB(); }

    static Client client;
};

void REQUEST_TESTS::SetUpTestCase() {
    clearDB();
#if defined(CREATE_SERVER)
    run_server();
#endif
    client.connect();
}
void REQUEST_TESTS::TearDownTestCase() {
#if defined(CREATE_SERVER)
    stop_server();
#endif
}

Client REQUEST_TESTS::client("localhost", 11234);

TEST_F(REQUEST_TESTS, CREATE_TABLE_TEST_1) {
    CHECK_REQUEST_ST_CLIENT("create table a (b int);", 0, "");
    CHECK_REQUEST_ST_CLIENT("show create table a;", 0,
                            "CREATE TABLE a(\n    b int\n);\n");
}

TEST_F(REQUEST_TESTS, CREATE_TABLE_TEST_2) {
    CHECK_REQUEST_ST_CLIENT(
        "create table a (b int)", EXCEPTION2NUMB(exc::ExceptionType::syntax),
        "~~Exception 1:\n wrong syntax!\n~~Exception in command:\"create "
        "table a (b int)\"\n");
}

TEST_F(REQUEST_TESTS, CREATE_TABLE_TEST_3) {
    CHECK_REQUEST_ST_CLIENT("create table a(b int not null);", 0, "");
}

TEST_F(REQUEST_TESTS, CREATE_TABLE_TEST_4) {
    CHECK_REQUEST_ST_CLIENT(
        "create table a(b int not null not null);",
        EXCEPTION2NUMB(exc::ExceptionType::redundant_constraints),
        "~~Exception 802:\n duplicate of constraint b in column a.\n"
        "~~Exception in command:\"create table a(b int not null not "
        "null);\"\n");
}

TEST_F(REQUEST_TESTS, CREATE_TABLE_TEST_5) {
    CHECK_REQUEST_ST_CLIENT(
        "create table a(b int not null, c real unique, d "
        "varchar(100) primary key);",
        0, "");
    CHECK_REQUEST_ST_CLIENT("show create table a;", 0,
                            "CREATE TABLE a(\n"
                            "    b int not null,\n"
                            "    c real unique,\n"
                            "    d varchar(100) primary key\n);\n");
}

TEST_F(REQUEST_TESTS, CREATE_TABLE_TEST_6) {
    CHECK_REQUEST_ST_CLIENT("create table sfs;",
                            EXCEPTION2NUMB(exc::ExceptionType::syntax),
                            "~~Exception 1:\n wrong syntax!\n"
                            "~~Exception in command:\"create table sfs;\"\n");
}

TEST_F(REQUEST_TESTS, CREATE_TABLE_TEST_7) {
    CHECK_REQUEST_ST_CLIENT("create table a(i int);", 0, "");
    CHECK_REQUEST_ST_CLIENT("create table b(i int);", 0, "");
    CHECK_REQUEST_ST_CLIENT("create table c(i int);", 0, "");
    CHECK_REQUEST_ST_CLIENT("show create table a;", 0,
                            "CREATE TABLE a(\n    i int\n);\n");
    CHECK_REQUEST_ST_CLIENT("show create table b;", 0,
                            "CREATE TABLE b(\n    i int\n);\n");
    CHECK_REQUEST_ST_CLIENT("show create table c;", 0,
                            "CREATE TABLE c(\n    i int\n);\n");
}

TEST_F(REQUEST_TESTS, CREATE_TABLE_TEST_8) {
    CHECK_REQUEST_ST_CLIENT(
        "create table a(MyColumn int, mycolumn varchar(100));",
        EXCEPTION2NUMB(exc::ExceptionType::repeat_column_in_table),
        "~~Exception 5:\n repeat column MyColumn in table a.\n"
        "~~Exception in command:\"create table a(MyColumn int, "
        "mycolumn varchar(100));\"\n");
}

TEST_F(REQUEST_TESTS, CREATE_TABLE_TEST_9) {
    CHECK_REQUEST_ST_CLIENT("create table a(MyColumn int, q varchar(100));", 0,
                            "");
    CHECK_REQUEST_ST_CLIENT(
        "create table a(f int);",
        EXCEPTION2NUMB(exc::ExceptionType::create_table_repeat_table_name),
        "~~Exception 1002 in create table a:\n this table name is "
        "repeated!\n~~Exception in command:\"create table a(f int);\"\n");
}

TEST_F(REQUEST_TESTS, CREATE_TABLE_TEST_10) {
    CHECK_REQUEST_ST_CLIENT(
        "create table 1a(f int);", EXCEPTION2NUMB(exc::ExceptionType::syntax),
        "~~Exception 1:\n wrong syntax!\n"
        "~~Exception in command:\"create table 1a(f int);\"\n");
}

TEST_F(REQUEST_TESTS, CREATE_TABLE_TEST_11) {
    CHECK_REQUEST_ST_CLIENT(
        "create table a(1f int);", EXCEPTION2NUMB(exc::ExceptionType::syntax),
        "~~Exception 1:\n wrong syntax!\n"
        "~~Exception in command:\"create table a(1f int);\"\n");
}

TEST_F(REQUEST_TESTS, CREATE_TABLE_TEST_12) {
    CHECK_REQUEST_ST_CLIENT(
        "create table a(b varchar(100) not null unique primary key);", 0, "");
    CHECK_REQUEST_ST_CLIENT(
        "show create table a;", 0,
        "CREATE TABLE a(\n    b varchar(100) primary key not null "
        "unique\n);\n");
}

TEST_F(REQUEST_TESTS, CREATE_TABLE_TEST_13) {
    CHECK_REQUEST_ST_CLIENT(
        "create table a(b int primary key, c int primary key);",
        EXCEPTION2NUMB(exc::ExceptionType::duplicated_primary_key_in_column),
        "~~Exception 803 in table a:\n primary key is used in the column b "
        "and in c.\n"
        "~~Exception in command:\"create table a(b int primary key, c int "
        "primary key);\"\n");
}

TEST_F(REQUEST_TESTS, SHOW_CREATE_TABLE_TEST_1) {
    CHECK_REQUEST_ST_CLIENT("show ghgh;",
                            EXCEPTION2NUMB(exc::ExceptionType::syntax),
                            "~~Exception 1:\n wrong syntax!\n"
                            "~~Exception in command:\"show ghgh;\"\n");
}

TEST_F(REQUEST_TESTS, SHOW_CREATE_TABLE_TEST_2) {
    CHECK_REQUEST_ST_CLIENT(
        "show create table e;",
        EXCEPTION2NUMB(exc::ExceptionType::access_table_nonexistent),
        "\n~~Exception 701:\n table e nonexistent.\n~~Exception in "
        "command:\"show create table e;\"\n");
}

TEST_F(REQUEST_TESTS, SYNTAX_TEST_1) {
    CHECK_REQUEST_ST_CLIENT("CreAte    \n  TablE   NamE \n ( A ReAl);", 0, "");
}

TEST_F(REQUEST_TESTS, SYNTAX_TEST_2) {
    CHECK_REQUEST_ST_CLIENT("fdgd;", EXCEPTION2NUMB(exc::ExceptionType::syntax),
                            "~~Exception 1:\n wrong syntax!\n~~Exception in "
                            "command:\"fdgd;\"\n");
}

TEST_F(REQUEST_TESTS, SYNTAX_TEST_3) {
    CHECK_REQUEST_ST_CLIENT("CREATE TABLE TAB(F VARCHAR(100));", 0, "");
}

TEST_F(REQUEST_TESTS, SYNTAX_TEST_4) {
    CHECK_REQUEST_ST_CLIENT("create table a(b varchar(100));", 0, "");
    CHECK_REQUEST_ST_CLIENT("insert into a values ('\'' ;');", 0, "");
}

TEST_F(REQUEST_TESTS, DROP_TABLE_TEST_1) {
    CHECK_REQUEST_ST_CLIENT(
        "drop table tr;",
        EXCEPTION2NUMB(exc::ExceptionType::access_table_nonexistent),
        "~~Exception 701:\n table tr nonexistent."
        "\n~~Exception in command:\"drop table tr;\"\n");
}

TEST_F(REQUEST_TESTS, DROP_TABLE_TEST_2) {
    CHECK_REQUEST_ST_CLIENT("create table a(b int);", 0, "");
    CHECK_REQUEST_ST_CLIENT("create table b(c real);", 0, "");
    CHECK_REQUEST_ST_CLIENT("drop table a;", 0, "");
    CHECK_REQUEST_ST_CLIENT("create table a(c varchar(100));", 0, "");
    CHECK_REQUEST_ST_CLIENT("show create table b;", 0,
                            "CREATE TABLE b(\n    c real\n);\n");
}

TEST_F(REQUEST_TESTS, DROP_TABLE_TEST_3) {
    CHECK_REQUEST_ST_CLIENT("drop table;",
                            EXCEPTION2NUMB(exc::ExceptionType::syntax),
                            "~~Exception 1:\n wrong syntax!\n"
                            "~~Exception in command:\"drop table;\"\n");
}

TEST_F(REQUEST_TESTS, SELECT_TEST_1) {
    CHECK_REQUEST_ST_CLIENT("create table a(a int, b real, c varchar(100));", 0,
                            "");
    CHECK_REQUEST_ST_CLIENT("select * from a;", 0, "");
}

TEST_F(REQUEST_TESTS, SELECT_TEST_2) {
    CHECK_REQUEST_ST_CLIENT("create table a(a int, b int, c int);", 0, "");
    CHECK_REQUEST_ST_CLIENT("select *, c, a from a;", 0, "");
}

TEST_F(REQUEST_TESTS, SELECT_TEST_3) {
    CHECK_REQUEST_ST_CLIENT("create table a(a int, b real, c varchar(100));", 0,
                            "");
    CHECK_REQUEST_ST_CLIENT("select c, * from a;",
                            EXCEPTION2NUMB(exc::ExceptionType::syntax),
                            "~~Exception 1:\n wrong syntax!\n"
                            "~~Exception in command:\"select c, * from a;\"\n");
}

TEST_F(REQUEST_TESTS, SELECT_TEST_4) {
    CHECK_REQUEST_ST_CLIENT(
        "select * from a;",
        EXCEPTION2NUMB(exc::ExceptionType::access_table_nonexistent),
        "~~Exception 701:\n table a nonexistent.\n"
        "~~Exception in command:\"select * from a;\"\n");
}

TEST_F(REQUEST_TESTS, SELECT_TEST_5) {
    CHECK_REQUEST_ST_CLIENT("create table a(a int, b real, c varchar(100));", 0,
                            "");
    CHECK_REQUEST_ST_CLIENT("select *, c, a, f from a;", 0, "");
}

TEST_F(REQUEST_TESTS, SELECT_TEST_6) {
    CHECK_REQUEST_ST_CLIENT("create table a(a int, b real, c varchar(100));", 0,
                            "");
    CHECK_REQUEST_ST_CLIENT("drop table a;", 0, "");
    CHECK_REQUEST_ST_CLIENT(
        "select * from a;",
        EXCEPTION2NUMB(exc::ExceptionType::access_table_nonexistent),
        "~~Exception 701:\n table a nonexistent.\n"
        "~~Exception in command:\"select * from a;\"\n");
}

TEST_F(REQUEST_TESTS, SELECT_TEST_7) {
    CHECK_REQUEST_ST_CLIENT("create table a(a int, b real, c varchar(100));", 0,
                            "");
    CHECK_REQUEST_ST_CLIENT("insert into a values (1, 0, '1');", 0, "");
    CHECK_REQUEST_ST_CLIENT("insert into a values (1, 1, '0');", 0, "");
    CHECK_REQUEST_ST_CLIENT("insert into a values (0, 1, '1');", 0, "");
    CHECK_REQUEST_ST_CLIENT("select * from a where a = 0;", 0,
                            "a: 0\nb: 1.000000\nc: 1\n");
}

TEST_F(REQUEST_TESTS, SELECT_TEST_8) {
    CHECK_REQUEST_ST_CLIENT("create table a(a int);", 0, "");
    CHECK_REQUEST_ST_CLIENT("insert into a values (1);", 0, "");
    CHECK_REQUEST_ST_CLIENT(
        "select * from a where a = '1';",
        EXCEPTION2NUMB(exc::ExceptionType::compare_data_type_mismatch),
        "~~Exception 605:\n can't compare int and varchar.\n~~Exception in "
        "command:\"select * from a where a = '1';\"\n");
}

TEST_F(REQUEST_TESTS, INSERT_TEST_1) {
    CHECK_REQUEST_ST_CLIENT("create table a(a int, b real, c varchar(100));", 0,
                            "");
    CHECK_REQUEST_ST_CLIENT("insert into a values (3, 2.2, 'Hello');", 0, "");
    CHECK_REQUEST_ST_CLIENT("select * from a;", 0,
                            "a: 3\nb: 2.200000\nc: Hello\n");
}

TEST_F(REQUEST_TESTS, INSERT_TEST_2) {
    CHECK_REQUEST_ST_CLIENT("create table a(a int, b real, c varchar(100));", 0,
                            "");
    CHECK_REQUEST_ST_CLIENT("insert into a (c, a, b) values ('Hello', 3, 2.2);",
                            0, "");
    CHECK_REQUEST_ST_CLIENT("select * from a;", 0,
                            "a: 3\nb: 2.200000\nc: Hello\n");
}

TEST_F(REQUEST_TESTS, INSERT_TEST_3) {
    CHECK_REQUEST_ST_CLIENT("create table a(a int, b real, c varchar(100));", 0,
                            "");
    CHECK_REQUEST_ST_CLIENT(
        "insert into a values (2.2, 3.3, 'Hello');",
        EXCEPTION2NUMB(exc::ExceptionType::set_data_type_mismatch),
        "~~Exception 601:\n value a is not compatible with data type int.\n"
        "~~Exception in command:\"insert into a values (2.2, 3.3, "
        "'Hello');\"\n");
}

TEST_F(REQUEST_TESTS, INSERT_TEST_4) {
    CHECK_REQUEST_ST_CLIENT("create table a(a int, b real, c varchar(100));", 0,
                            "");
    CHECK_REQUEST_ST_CLIENT("insert into a values (-2, -1, '');", 0, "");
    CHECK_REQUEST_ST_CLIENT("select * from a;", 0,
                            "a: -2\nb: -1.000000\nc: \n");
}

TEST_F(REQUEST_TESTS, INSERT_TEST_5) {
    CHECK_REQUEST_ST_CLIENT("create table a(a int, b real, c varchar(100));", 0,
                            "");
    CHECK_REQUEST_ST_CLIENT(
        "insert into a values (2, 3.3.3, 'Hello');",
        EXCEPTION2NUMB(exc::ExceptionType::syntax),
        "~~Exception 1:\n wrong syntax!\n"
        "~~Exception in command:\"insert into a values (2, 3.3.3, "
        "'Hello');\"\n");
}

TEST_F(REQUEST_TESTS, INSERT_TEST_6) {
    CHECK_REQUEST_ST_CLIENT("create table a(a int, b real, c varchar(100));", 0,
                            "");
    CHECK_REQUEST_ST_CLIENT(
        "insert into a values ('Hello', 3.3, 'Hello');",
        EXCEPTION2NUMB(exc::ExceptionType::set_data_type_mismatch),
        "~~Exception 601:\n value a is not compatible with data type int.\n"
        "~~Exception in command:\"insert into a values ('Hello', 3.3, "
        "'Hello');\"\n");
}

TEST_F(REQUEST_TESTS, INSERT_TEST_7) {
    CHECK_REQUEST_ST_CLIENT("create table a(a int, b real, c varchar(100));", 0,
                            "");
    CHECK_REQUEST_ST_CLIENT(
        "insert into a values (2, 3.3, 4);",
        EXCEPTION2NUMB(exc::ExceptionType::set_data_type_mismatch),
        "~~Exception 601:\n value c is not compatible with data "
        "type varchar.\n~~Exception in command:\"insert into a "
        "values (2, 3.3, 4);\"\n");
}

TEST_F(REQUEST_TESTS, INSERT_TEST_8) {
    CHECK_REQUEST_ST_CLIENT("create table a(a int, b real, c varchar(100));", 0,
                            "");
    CHECK_REQUEST_ST_CLIENT(
        "insert into a(b, a, c) values (2, 3.3, 'Hello');",
        EXCEPTION2NUMB(exc::ExceptionType::set_data_type_mismatch),
        "~~Exception 601:\n value a is not compatible with data type "
        "int.\n~~Exception in command:\"insert into a(b, a, c) values (2, "
        "3.3, "
        "'Hello');\"\n");
}

TEST_F(REQUEST_TESTS, INSERT_TEST_9) {
    CHECK_REQUEST_ST_CLIENT(
        "insert into a values (2, 3.3, 'Hello');",
        EXCEPTION2NUMB(exc::ExceptionType::access_table_nonexistent),
        "~~Exception 701:\n table a nonexistent.\n~~Exception in "
        "command:\"insert into a values (2, 3.3, 'Hello');\"\n");
}

TEST_F(REQUEST_TESTS, INSERT_TEST_10) {
    CHECK_REQUEST_ST_CLIENT("create table a(a int, b real, c varchar(100));", 0,
                            "");
    CHECK_REQUEST_ST_CLIENT(
        "insert into a(a, b, f) values (2, 3.3, 'Hello');",
        EXCEPTION2NUMB(exc::ExceptionType::access_column_nonexistent),
        "~~Exception 702:\n column f in table a nonexistent.\n"
        "~~Exception in command:\"insert into a(a, b, f) values (2, 3.3, "
        "'Hello');\"\n");
}

TEST_F(REQUEST_TESTS, INSERT_TEST_11) {
    CHECK_REQUEST_ST_CLIENT("create table a(a int, b real, c varchar(100));", 0,
                            "");
    CHECK_REQUEST_ST_CLIENT("insert into a(a) values (-2);", 0, "");
    CHECK_REQUEST_ST_CLIENT("select * from a;", 0, "a: -2\nb: null\nc: \n");
}

TEST_F(REQUEST_TESTS, INSERT_TEST_12) {
    CHECK_REQUEST_ST_CLIENT("create table a(a int, b real, c varchar(100));", 0,
                            "");
    CHECK_REQUEST_ST_CLIENT("insert into a(a) values (8);", 0, "");
    CHECK_REQUEST_ST_CLIENT("select * from a;", 0, "a: 8\nb: null\nc: \n");
}

TEST_F(REQUEST_TESTS, INSERT_TEST_13) {
    CHECK_REQUEST_ST_CLIENT(
        "create table a(a int not null, b real primary key, c varchar(100) "
        "unique);",
        0, "");
    CHECK_REQUEST_ST_CLIENT("insert into a values (-2, 0.1, 'Hello world!');",
                            0, "");
    CHECK_REQUEST_ST_CLIENT("select * from a;", 0,
                            "a: -2\nb: 0.100000\nc: Hello world!\n");
}

TEST_F(REQUEST_TESTS, INSERT_TEST_14) {
    CHECK_REQUEST_ST_CLIENT(
        "create table a(a int not null, b real primary key, c varchar(100) "
        "unique);",
        0, "");
    CHECK_REQUEST_ST_CLIENT(
        "insert into a(b, c) values (0.1, 'H M!');",
        EXCEPTION2NUMB(exc::ExceptionType::null_not_null),
        "~~Exception 805 in table a:\n a can't contain null values.\n"
        "~~Exception in command:\"insert into a(b, c) values (0.1, 'H "
        "M!');\"\n");
}

TEST_F(REQUEST_TESTS, INSERT_TEST_15) {
    CHECK_REQUEST_ST_CLIENT(
        "create table a(a int not null, b real primary key, c varchar(100) "
        "unique);",
        0, "");
    CHECK_REQUEST_ST_CLIENT("insert into a values (1, 0, 'H M!');", 0, "");
    CHECK_REQUEST_ST_CLIENT(
        "insert into a values (12, 0, 'H!');",
        EXCEPTION2NUMB(exc::ExceptionType::duplicated_unique),
        "~~Exception 804 in table a:\n 0.000000 is not unique is in the "
        "column "
        "b.\n~~Exception in command:\"insert into a values (12, 0, "
        "'H!');\"\n");
}

TEST_F(REQUEST_TESTS, INSERT_TEST_16) {
    CHECK_REQUEST_ST_CLIENT(
        "create table a(a int not null, b real primary key, c "
        "varchar(100) unique);",
        0, "");
    CHECK_REQUEST_ST_CLIENT("insert into a values (1, 0, 'H!');", 0, "");
    CHECK_REQUEST_ST_CLIENT(
        "insert into a values (12, 1, 'H!');",
        EXCEPTION2NUMB(exc::ExceptionType::duplicated_unique),
        "~~Exception 804 in table a:\n null is not unique is in "
        "the column c.\n~~Exception in command:\"insert into a "
        "values (12, 1, 'H!');\"\n");
}

TEST_F(REQUEST_TESTS, INSERT_TEST_17) {
    CHECK_REQUEST_ST_CLIENT(
        "create table a(a int not null, b real primary key, c varchar(100) "
        "unique);",
        0, "");
    CHECK_REQUEST_ST_CLIENT("insert into a values (1, 0, null);", 0, "");
    CHECK_REQUEST_ST_CLIENT(
        "insert into a(a, b) values (12, 1);",
        EXCEPTION2NUMB(exc::ExceptionType::duplicated_unique),
        "~~Exception 804 in table a:\n null is not unique is in the column "
        "c.\n~~Exception in command:\"insert into a(a, b) values (12, "
        "1);\"\n");
}

TEST_F(REQUEST_TESTS, INSERT_TEST_18) {
    CHECK_REQUEST_ST_CLIENT(
        "create table a(a int not null, b real primary key, c "
        "varchar(100) unique);",
        0, "");
    CHECK_REQUEST_ST_CLIENT("insert into a values (1, 0, 'H!');", 0, "");
    CHECK_REQUEST_ST_CLIENT(
        "insert into a(a, b) values (12, 1);"
        "select * from a;",
        0, "a: 1\nb: 0.000000\nc: H!\na: 12\nb: 1.000000\nc: \n");
}

TEST_F(REQUEST_TESTS, INSERT_TEST_19) {
    CHECK_REQUEST_ST_CLIENT("create table a(a int, b real, c varchar(100));", 0,
                            "");
    CHECK_REQUEST_ST_CLIENT("insert into a(a, a) values (12, 1);",
                            EXCEPTION2NUMB(exc::ExceptionType::repeat_column),
                            "~~Exception 6:\n repeat column a.\n~~Exception in "
                            "command:\"insert "
                            "into a(a, a) values (12, 1);\"\n");
}

TEST_F(REQUEST_TESTS, INSERT_TEST_20) {
    CHECK_REQUEST_ST_CLIENT("create table a(a int, b real, c varchar(100));", 0,
                            "");
    CHECK_REQUEST_ST_CLIENT(
        "insert into a(a) values (12, 1);",
        EXCEPTION2NUMB(exc::ExceptionType::insert_constants_more_columns),
        "~~Exception 1101:\n the number of constants is more than "
        "columns.\n"
        "~~Exception in command:\"insert into a(a) values (12, 1);\"\n");
}

TEST_F(REQUEST_TESTS, INSERT_TEST_21) {
    CHECK_REQUEST_ST_CLIENT("create table a(a int, b real, c varchar(100));", 0,
                            "");
    CHECK_REQUEST_ST_CLIENT(
        "insert into a values (12, 12, ' ') where b = 2 + 1;",
        EXCEPTION2NUMB(exc::ExceptionType::syntax),
        "~~Exception 1:\n wrong syntax!\n"
        "~~Exception in command:\"insert into a values (12, 12, ' ') where "
        "b = "
        "2 + 1;\"\n");
}

TEST_F(REQUEST_TESTS, DELETE_TEST_1) {
    CHECK_REQUEST_ST_CLIENT("create table a(a int, b real, c varchar(100));", 0,
                            "");
    CHECK_REQUEST_ST_CLIENT("insert into a values (1, 0, '1');", 0, "");
    CHECK_REQUEST_ST_CLIENT("insert into a values (1, 1, '0');", 0, "");
    CHECK_REQUEST_ST_CLIENT("insert into a values (0, 1, '1');", 0, "");
    CHECK_REQUEST_ST_CLIENT("delete from a where a = 0;", 0, "");
    CHECK_REQUEST_ST_CLIENT(
        "select * from a;", 0,
        "a: 1\nb: 0.000000\nc: 1\na: 1\nb: 1.000000\nc: 0\n");
    CHECK_REQUEST_ST_CLIENT("delete from a where b = 0;", 0, "");
    CHECK_REQUEST_ST_CLIENT("select * from a;", 0, "a: 1\nb: 1.000000\nc: 0\n");
    CHECK_REQUEST_ST_CLIENT("delete from a where c = '0';", 0, "");
    CHECK_REQUEST_ST_CLIENT("select * from a;", 0, "");
}

TEST_F(REQUEST_TESTS, DELETE_TEST_2) {
    CHECK_REQUEST_ST_CLIENT("create table a(a int, b real, c varchar(100));", 0,
                            "");
    CHECK_REQUEST_ST_CLIENT("delete from a where c = '0';", 0, "");
    CHECK_REQUEST_ST_CLIENT("select * from a;", 0, "");
}

TEST_F(REQUEST_TESTS, DELETE_TEST_3) {
    CHECK_REQUEST_ST_CLIENT(
        "delete from a where c = '0';",
        EXCEPTION2NUMB(exc::ExceptionType::access_table_nonexistent),
        "~~Exception 701:\n table a nonexistent.\n~~Exception in "
        "command:\"delete from a where c = '0';\"\n");
}

TEST_F(REQUEST_TESTS, DELETE_TEST_4) {
    CHECK_REQUEST_ST_CLIENT("create table a(a int, b real, c varchar(100));", 0,
                            "");
    CHECK_REQUEST_ST_CLIENT(
        "delete from a where f = '0';",
        0,
        "");
}

TEST_F(REQUEST_TESTS, DELETE_TEST_5) {
    CHECK_REQUEST_ST_CLIENT("create table a(a int, b real, c varchar(100));", 0,
                            "");
    CHECK_REQUEST_ST_CLIENT(
        "delete from a where b = '0';",
        0,
        "");
}

TEST_F(REQUEST_TESTS, DELETE_TEST_6) {
    CHECK_REQUEST_ST_CLIENT("create table a(a int, b real, c varchar(100));", 0,
                            "");
    CHECK_REQUEST_ST_CLIENT("insert into a values (1, 0, '1');", 0, "");
    CHECK_REQUEST_ST_CLIENT("insert into a values (1, 1, '0');", 0, "");
    CHECK_REQUEST_ST_CLIENT("insert into a values (0, 1, '1');", 0, "");
    CHECK_REQUEST_ST_CLIENT("insert into a values (0, 0, '1');", 0, "");
    CHECK_REQUEST_ST_CLIENT("delete from a where b = a or a = 1;", 0, "");
    CHECK_REQUEST_ST_CLIENT("select a from a;", 0, "a: 0\n");
}

TEST_F(REQUEST_TESTS, UPDATE_TEST_1) {
    CHECK_REQUEST_ST_CLIENT("create table a(a int, b real, c varchar(100));", 0,
                            "");
    CHECK_REQUEST_ST_CLIENT("insert into a values (1, 0, '1');", 0, "");
    CHECK_REQUEST_ST_CLIENT("insert into a values (1, 1, '0');", 0, "");
    CHECK_REQUEST_ST_CLIENT("insert into a values (0, 1, '1');", 0, "");
    CHECK_REQUEST_ST_CLIENT("update a set a = 2;", 0, "");
    CHECK_REQUEST_ST_CLIENT(
        "select * from a;", 0,
        "a: 2\nb: 1.000000\nc: 1\na: 2\nb: 1.000000\nc: 1\na: "
        "2\nb: 1.000000\nc: 1\n");
    CHECK_REQUEST_ST_CLIENT("update a set b = 3.45, c = 'H';", 0, "");
    CHECK_REQUEST_ST_CLIENT(
        "select * from a;", 0,
        "a: 2\nb: 3.450000\nc: H\na: 2\nb: 3.450000\nc: H\na: "
        "2\nb: 3.450000\nc: H\n");
}

TEST_F(REQUEST_TESTS, UPDATE_TEST_2) {
    CHECK_REQUEST_ST_CLIENT("create table a(a int, b real, c varchar(100));", 0,
                            "");
    CHECK_REQUEST_ST_CLIENT("insert into a values (1, 0, '1');", 0, "");
    CHECK_REQUEST_ST_CLIENT("insert into a values (1, 1, '0');", 0, "");
    CHECK_REQUEST_ST_CLIENT("insert into a values (0, 1, '1');", 0, "");
    CHECK_REQUEST_ST_CLIENT(
        "update a set f = 2;",
        EXCEPTION2NUMB(exc::ExceptionType::access_column_nonexistent),
        "~~Exception 702:\n column f in table a nonexistent.\n~~Exception "
        "in "
        "command:\"update a set f = 2;\"\n");
}

TEST_F(REQUEST_TESTS, UPDATE_TEST_3) {
    CHECK_REQUEST_ST_CLIENT(
        "update a set f = 2;",
        EXCEPTION2NUMB(exc::ExceptionType::access_table_nonexistent),
        "~~Exception 701:\n table a nonexistent.\n~~Exception in "
        "command:\"update a set f = 2;\"\n");
}

TEST_F(REQUEST_TESTS, UPDATE_TEST_4) {
    CHECK_REQUEST_ST_CLIENT("create table a(a int unique);", 0, "");
    CHECK_REQUEST_ST_CLIENT("insert into a values (1);", 0, "");
    CHECK_REQUEST_ST_CLIENT("insert into a values (2);", 0, "");
    CHECK_REQUEST_ST_CLIENT(
        "update a set a = 2;",
        EXCEPTION2NUMB(exc::ExceptionType::duplicated_unique),
        "~~Exception 804 in table a:\n 2 is not unique is in the column "
        "a.\n~~Exception in command:\"update a set a = 2;\"\n");
}

TEST_F(REQUEST_TESTS, UPDATE_TEST_5) {
    CHECK_REQUEST_ST_CLIENT("create table a(a int not null);", 0, "");
    CHECK_REQUEST_ST_CLIENT("insert into a values (1);", 0, "");
    CHECK_REQUEST_ST_CLIENT("insert into a values (2);", 0, "");
    CHECK_REQUEST_ST_CLIENT(
        "update a set a = null;",
        EXCEPTION2NUMB(exc::ExceptionType::null_not_null),
        "~~Exception 805 in table a:\n a can't contain null "
        "values.\n~~Exception in command:\"update a set a = null;\"\n");
}

TEST_F(REQUEST_TESTS, UPDATE_TEST_6) {
    CHECK_REQUEST_ST_CLIENT("create table a(a int primary key);", 0, "");
    CHECK_REQUEST_ST_CLIENT("insert into a values (1);", 0, "");
    CHECK_REQUEST_ST_CLIENT("insert into a values (2);", 0, "");
    CHECK_REQUEST_ST_CLIENT(
        "update a set a = 2;",
        EXCEPTION2NUMB(exc::ExceptionType::duplicated_unique),
        "~~Exception 804 in table a:\n 2 is not unique is in the column "
        "a.\n~~Exception in command:\"update a set a = 2;\"\n");
}

TEST_F(REQUEST_TESTS, UPDATE_TEST_7) {
    CHECK_REQUEST_ST_CLIENT("create table a(a int primary key);", 0, "");
    CHECK_REQUEST_ST_CLIENT("insert into a values (1);", 0, "");
    CHECK_REQUEST_ST_CLIENT("insert into a values (2);", 0, "");
    CHECK_REQUEST_ST_CLIENT(
        "update a set a = '2';",
        EXCEPTION2NUMB(exc::ExceptionType::set_data_type_mismatch),
        "~~Exception 601:\n value a is not compatible with data type "
        "int.\n~~Exception in command:\"update a set a = '2';\"\n");
}

TEST_F(REQUEST_TESTS, WHERE_TEST_1) {
    CHECK_REQUEST_ST_CLIENT("create table a (a varchar(100), b varchar(100));",
                            0, "");
    CHECK_REQUEST_ST_CLIENT("insert into a values('ab', 'ab');", 0, "");
    CHECK_REQUEST_ST_CLIENT("select * from a where '1';", 0, "a: ab\nb: ab\n");
    CHECK_REQUEST_ST_CLIENT("insert into a values('ab', 'abc');", 0, "");
    CHECK_REQUEST_ST_CLIENT("insert into a values('ab', 'b');", 0, "");
    CHECK_REQUEST_ST_CLIENT("insert into a values('ab', 'bb');", 0, "");
    CHECK_REQUEST_ST_CLIENT("insert into a values('ab', 'a');", 0, "");
    CHECK_REQUEST_ST_CLIENT("insert into a values('ab', 'aa');", 0, "");
    CHECK_REQUEST_ST_CLIENT("insert into a values('ab', '');", 0, "");
    CHECK_REQUEST_ST_CLIENT("select * from a where a < b;", 0,
                            "a: ab\nb: abc\na: ab\nb: b\na: ab\nb: bb\n");
    CHECK_REQUEST_ST_CLIENT(
        "select * from a where a <= b;", 0,
        "a: ab\nb: ab\na: ab\nb: abc\na: ab\nb: b\na: ab\nb: bb\n");
    CHECK_REQUEST_ST_CLIENT("select * from a where a = b;", 0,
                            "a: ab\nb: ab\n");
    CHECK_REQUEST_ST_CLIENT(
        "select * from a where a >= b;", 0,
        "a: ab\nb: ab\na: ab\nb: a\na: ab\nb: aa\na: ab\nb: \n");
    CHECK_REQUEST_ST_CLIENT("select * from a where a > b;", 0,
                            "a: ab\nb: a\na: ab\nb: aa\na: ab\nb: \n");
    CHECK_REQUEST_ST_CLIENT(
        "select * from a where a >= b and b > 'aa' or b = 'b';", 0,
        "a: ab\nb: ab\na: ab\nb: b\n");
    CHECK_REQUEST_ST_CLIENT("select * from a where a < b and b != 'abc';", 0,
                            "a: ab\nb: b\na: ab\nb: bb\n");
    CHECK_REQUEST_ST_CLIENT("select * from a where a + 'b' = 'abb';",
                            exc::ExceptionType::no_operation_for_type,
                            "~~Exception 603:\n no operation '+' for varchar "
                            "and varchar.\n~~Exception in command:\"select * "
                            "from a where a + 'b' = 'abb';\"\n");
    CHECK_REQUEST_ST_CLIENT("select * from a where a - 'b' = 'a';",
                            exc::ExceptionType::no_operation_for_type,
                            "~~Exception 603:\n no operation '-' for varchar "
                            "and varchar.\n~~Exception in command:\"select * "
                            "from a where a - 'b' = 'a';\"\n");
    CHECK_REQUEST_ST_CLIENT(
        "select * from a where a * 2 = 'abb';",
        exc::ExceptionType::no_operation_for_type,
        "~~Exception 603:\n no operation '*' for varchar and "
        "int.\n~~Exception "
        "in command:\"select * from a where a * 2 = 'abb';\"\n");
    CHECK_REQUEST_ST_CLIENT("select * from a where a / 'b' = 'abb';",
                            exc::ExceptionType::no_operation_for_type,
                            "~~Exception 603:\n no operation '/' for varchar "
                            "and varchar.\n~~Exception in command:\"select * "
                            "from a where a / 'b' = 'abb';\"\n");
}

TEST_F(REQUEST_TESTS, WHERE_TEST_2) {
    CHECK_REQUEST_ST_CLIENT("create table a (a int, b int);", 0, "");
    CHECK_REQUEST_ST_CLIENT("insert into a values(-3, 3);", 0, "");
    CHECK_REQUEST_ST_CLIENT("select * from a where 1;", 0, "a: -3\nb: 3\n");
    CHECK_REQUEST_ST_CLIENT("select * from a where 0;", 0, "");
    CHECK_REQUEST_ST_CLIENT("insert into a values(-1, -1);", 0, "");
    CHECK_REQUEST_ST_CLIENT("insert into a values(-1, -10);", 0, "");
    CHECK_REQUEST_ST_CLIENT("insert into a values(-1, 10);", 0, "");
    CHECK_REQUEST_ST_CLIENT("select * from a where a < b;", 0,
                            "a: -3\nb: 3\na: -1\nb: 10\n");
    CHECK_REQUEST_ST_CLIENT("select * from a where a <= b;", 0,
                            "a: -3\nb: 3\na: -1\nb: -1\na: -1\nb: 10\n");
    CHECK_REQUEST_ST_CLIENT(  // TODO
        "select * from a where (12 + (4.5 * 2 / 3) + 2.0 * 2 - a * (-1)) / "
        "2 = "
        "b + a;",
        0, "a: -1\nb: 10\n");
    CHECK_REQUEST_ST_CLIENT("select * from a where a + 2 >= -0.567;", 0,
                            "a: -1\nb: -1\na: -1\nb: -10\na: -1\nb: 10\n");
    CHECK_REQUEST_ST_CLIENT(
        "select * from a where b != a and b != 10 and b != -10;", 0,
        "a: -3\nb: 3\n");
}

TEST_F(REQUEST_TESTS, WHERE_TEST_3) {
    CHECK_REQUEST_ST_CLIENT("create table a (a real, b real);", 0, "");
    CHECK_REQUEST_ST_CLIENT("insert into a values(-3.25, 3);", 0, "");
    CHECK_REQUEST_ST_CLIENT("select * from a where 0.0;", 0, "");
    CHECK_REQUEST_ST_CLIENT("select * from a where 0.12;", 0,
                            "a: -3.250000\nb: 3.000000\n");
    CHECK_REQUEST_ST_CLIENT("insert into a values(-1.0, -1);", 0, "");
    CHECK_REQUEST_ST_CLIENT("insert into a values(0.123, -10);", 0, "");
    CHECK_REQUEST_ST_CLIENT(
        "select * from a where a + b < (-1.234 * (-1) + 2.05 * 3 - 1.2 / "
        "2.1) / (-3.7);",
        0, "a: -1.000000\nb: -1.000000\na: 0.123000\nb: -10.000000\n");
    CHECK_REQUEST_ST_CLIENT(
        "select * from a where a  <= b;", 0,
        "a: -3.250000\nb: 3.000000\na: -1.000000\nb: -1.000000\n");
    CHECK_REQUEST_ST_CLIENT("select * from a where a / 3.25 * 3 = b * -1;", 0,
                            "a: -3.250000\nb: 3.000000\n");
    CHECK_REQUEST_ST_CLIENT(
        "select * from a where a >= b;", 0,
        "a: -1.000000\nb: -1.000000\na: 0.123000\nb: -10.000000\n");
    CHECK_REQUEST_ST_CLIENT("select * from a where a > 0;", 0,
                            "a: 0.123000\nb: -10.000000\n");
}

TEST_F(REQUEST_TESTS, WHERE_TEST_4) {
    CHECK_REQUEST_ST_CLIENT("create table a (a int, b real, c varchar(100));",
                            0, "");
    CHECK_REQUEST_ST_CLIENT("insert into a values(1, 2.35, '67 89');", 0, "");
    CHECK_REQUEST_ST_CLIENT("select * from a where a = b - 1.35;", 0,
                            "a: 1\nb: 2.350000\nc: 67 89\n");
    CHECK_REQUEST_ST_CLIENT(
        "select * from a where a <= c;",
        exc::ExceptionType::compare_data_type_mismatch,
        "~~Exception 605:\n can't compare int and varchar.\n~~Exception in "
        "command:\"select * from a where a <= c;\"\n");
    CHECK_REQUEST_ST_CLIENT("select * from a where b = c;",
                            exc::ExceptionType::compare_data_type_mismatch,
                            "~~Exception 605:\n can't compare real and "
                            "varchar.\n~~Exception in "
                            "command:\"select * from a where b = c;\"\n");
    CHECK_REQUEST_ST_CLIENT("select * from a where not not a = a;", 0,
                            "a: 1\nb: 2.350000\nc: 67 89\n");
    CHECK_REQUEST_ST_CLIENT("select * from a where not a != a;", 0,
                            "a: 1\nb: 2.350000\nc: 67 89\n");
    CHECK_REQUEST_ST_CLIENT("select * from a where (a = 1) < 3;", 0,
                            "a: 1\nb: 2.350000\nc: 67 89\n");
    CHECK_REQUEST_ST_CLIENT("select * from a where (a = 1) > (3 != 3);", 0,
                            "a: 1\nb: 2.350000\nc: 67 89\n");
    CHECK_REQUEST_ST_CLIENT("select * from a where (a = 1) <= (3 != 3);", 0,
                            "");
    CHECK_REQUEST_ST_CLIENT("select * from a where (a = 1) * (3 != 3);", 0, "");
    CHECK_REQUEST_ST_CLIENT("select * from a where (a = 1) + (3 != 3);", 0,
                            "a: 1\nb: 2.350000\nc: 67 89\n");
    CHECK_REQUEST_ST_CLIENT(
        "select * from a where (a = 1) / 0;", exc::ExceptionType::div_by_zero,
        "~~Exception 7:\n division by zero '1/0'.\n~~Exception in "
        "command:\"select * from a where (a = 1) / 0;\"\n");
    CHECK_REQUEST_ST_CLIENT("select * from a where (a = 1) - (3 != 3);", 0,
                            "a: 1\nb: 2.350000\nc: 67 89\n");
    CHECK_REQUEST_ST_CLIENT("insert into a values(3, 0, '');", 0, "");
    CHECK_REQUEST_ST_CLIENT("select * from a where c =;",
                            exc::ExceptionType::syntax,
                            "~~Exception 1:\n wrong syntax!\n~~Exception in "
                            "command:\"select * from a where c =;\"\n");
    CHECK_REQUEST_ST_CLIENT(
        "select * from a where not a < 1 or b < 2 and c = '';", 0,
        "a: 1\nb: 2.350000\nc: 67 89\na: 3\nb: 0.000000\nc: \n");
    CHECK_REQUEST_ST_CLIENT(
        "select * from a where not a < 1 or b > 2 and c = '';", 0,
        "a: 1\nb: 2.350000\nc: 67 89\na: 3\nb: 0.000000\nc: \n");
    CHECK_REQUEST_ST_CLIENT(
        "select * from a where a >= 1 and (b = 2 or c = '');", 0,
        "a: 3\nb: 0.000000\nc: \n");
    CHECK_REQUEST_ST_CLIENT("select * from a where a - 3 = b = 0;", 0,
                            "a: 1\nb: 2.350000\nc: 67 89\n");
    CHECK_REQUEST_ST_CLIENT("select * from a where (a - 3 = 0) * 3 = 0;", 0,
                            "a: 1\nb: 2.350000\nc: 67 89\n");
    CHECK_REQUEST_ST_CLIENT("select * from a where a - 3 = b * 1 = 0;", 0,
                            "a: 1\nb: 2.350000\nc: 67 89\n");
}
