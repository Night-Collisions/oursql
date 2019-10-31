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

    static ourSQL::client::Client client;
};

void REQUEST_TESTS::SetUpTestCase() {
    clearDB();
#if defined(CREATE_SERVER)
    Server::get()->run();
#endif
    client.connect();
}
void REQUEST_TESTS::TearDownTestCase() {
#if defined(CREATE_SERVER)
    Server::get()->stop();
#endif
}

ourSQL::client::Client REQUEST_TESTS::client(TEST_SERVER_HOST,
                                             TEST_SERVER_PORT);

class CREATE_TABLE_TESTS : public REQUEST_TESTS {};

TEST_F(CREATE_TABLE_TESTS, TEST_1) {
    CHECK_REQUEST_ST_CLIENT("create table a (b int);", 0, "");
    CHECK_REQUEST_ST_CLIENT("show create table a;", 0,
                            "CREATE TABLE a(\n    b int\n);\n");
}

TEST_F(CREATE_TABLE_TESTS, TEST_2) {
    CHECK_REQUEST_ST_CLIENT(
        "create table a (b int)", EXCEPTION2NUMB(exc::ExceptionType::syntax),
        "~~Exception 1:\n wrong syntax!\n~~Exception in command:\"create "
        "table a (b int)\"\n");
}

TEST_F(CREATE_TABLE_TESTS, TEST_3) {
    CHECK_REQUEST_ST_CLIENT("create table a(b int not null);", 0, "");
}

TEST_F(CREATE_TABLE_TESTS, TEST_4) {
    CHECK_REQUEST_ST_CLIENT(
        "create table a(b int not null not null);",
        EXCEPTION2NUMB(exc::ExceptionType::redundant_constraints),
        "~~Exception 802:\n duplicate of constraint b in column a.\n"
        "~~Exception in command:\"create table a(b int not null not "
        "null);\"\n");
}

TEST_F(CREATE_TABLE_TESTS, TEST_5) {
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

TEST_F(CREATE_TABLE_TESTS, TEST_6) {
    CHECK_REQUEST_ST_CLIENT("create table sfs;",
                            EXCEPTION2NUMB(exc::ExceptionType::syntax),
                            "~~Exception 1:\n wrong syntax!\n"
                            "~~Exception in command:\"create table sfs;\"\n");
}

TEST_F(CREATE_TABLE_TESTS, TEST_7) {
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

TEST_F(CREATE_TABLE_TESTS, TEST_8) {
    CHECK_REQUEST_ST_CLIENT(
        "create table a(MyColumn int, mycolumn varchar(100));",
        EXCEPTION2NUMB(exc::ExceptionType::repeat_column_in_table),
        "~~Exception 5:\n repeat column MyColumn in table a.\n"
        "~~Exception in command:\"create table a(MyColumn int, "
        "mycolumn varchar(100));\"\n");
}

TEST_F(CREATE_TABLE_TESTS, TEST_9) {
    CHECK_REQUEST_ST_CLIENT("create table a(MyColumn int, q varchar(100));", 0,
                            "");
    CHECK_REQUEST_ST_CLIENT(
        "create table a(f int);",
        EXCEPTION2NUMB(exc::ExceptionType::create_table_repeat_table_name),
        "~~Exception 1002 in create table a:\n this table name is "
        "repeated!\n~~Exception in command:\"create table a(f int);\"\n");
}

TEST_F(CREATE_TABLE_TESTS, TEST_10) {
    CHECK_REQUEST_ST_CLIENT(
        "create table 1a(f int);", EXCEPTION2NUMB(exc::ExceptionType::syntax),
        "~~Exception 1:\n wrong syntax!\n"
        "~~Exception in command:\"create table 1a(f int);\"\n");
}

TEST_F(CREATE_TABLE_TESTS, TEST_11) {
    CHECK_REQUEST_ST_CLIENT(
        "create table a(1f int);", EXCEPTION2NUMB(exc::ExceptionType::syntax),
        "~~Exception 1:\n wrong syntax!\n"
        "~~Exception in command:\"create table a(1f int);\"\n");
}

TEST_F(CREATE_TABLE_TESTS, TEST_12) {
    CHECK_REQUEST_ST_CLIENT(
        "create table a(b varchar(100) not null unique primary key);", 0, "");
    CHECK_REQUEST_ST_CLIENT(
        "show create table a;", 0,
        "CREATE TABLE a(\n    b varchar(100) primary key not null "
        "unique\n);\n");
}

TEST_F(CREATE_TABLE_TESTS, TEST_13) {
    CHECK_REQUEST_ST_CLIENT(
        "create table a(b int primary key, c int primary key);",
        EXCEPTION2NUMB(exc::ExceptionType::duplicated_primary_key_in_column),
        "~~Exception 803 in table a:\n primary key is used in the column b "
        "and in c.\n"
        "~~Exception in command:\"create table a(b int primary key, c int "
        "primary key);\"\n");
}

class SHOW_CREATE_TABLE_TESTS : public REQUEST_TESTS {};

TEST_F(SHOW_CREATE_TABLE_TESTS, TEST_1) {
    CHECK_REQUEST_ST_CLIENT("show ghgh;",
                            EXCEPTION2NUMB(exc::ExceptionType::syntax),
                            "~~Exception 1:\n wrong syntax!\n"
                            "~~Exception in command:\"show ghgh;\"\n");
}

TEST_F(SHOW_CREATE_TABLE_TESTS, TEST_2) {
    CHECK_REQUEST_ST_CLIENT(
        "show create table e;",
        EXCEPTION2NUMB(exc::ExceptionType::access_table_nonexistent),
        "\n~~Exception 701:\n table e nonexistent.\n~~Exception in "
        "command:\"show create table e;\"\n");
}

class SYNTAX_TESTS : public REQUEST_TESTS {};

TEST_F(SYNTAX_TESTS, TEST_1) {
    CHECK_REQUEST_ST_CLIENT("CreAte    \n  TablE   NamE \n ( A ReAl);", 0, "");
}

TEST_F(SYNTAX_TESTS, TEST_2) {
    CHECK_REQUEST_ST_CLIENT("fdgd;", EXCEPTION2NUMB(exc::ExceptionType::syntax),
                            "~~Exception 1:\n wrong syntax!\n~~Exception in "
                            "command:\"fdgd;\"\n");
}

TEST_F(SYNTAX_TESTS, TEST_3) {
    CHECK_REQUEST_ST_CLIENT("CREATE TABLE TAB(F VARCHAR(100));", 0, "");
}

TEST_F(SYNTAX_TESTS, TEST_4) {
    CHECK_REQUEST_ST_CLIENT("create table a(b varchar(100));", 0, "");
    CHECK_REQUEST_ST_CLIENT("insert into a values ('\'' ;');", 0, "");
}

TEST_F(SYNTAX_TESTS, TEST_5) {
    CHECK_REQUEST_ST_CLIENT("create # table a(b varchar(100));", 0, "");
}

TEST_F(SYNTAX_TESTS, TEST_6) {
    CHECK_REQUEST_ST_CLIENT(
        "create table table(b varchar(100));", 1,
        "~~Exception 1:\n wrong syntax!\n~~Exception in command:\"create table "
        "table(b varchar(100));\"\n");
}

class DROP_TABLE_TESTS : public REQUEST_TESTS {};

TEST_F(DROP_TABLE_TESTS, TEST_1) {
    CHECK_REQUEST_ST_CLIENT(
        "drop table tr;",
        EXCEPTION2NUMB(exc::ExceptionType::access_table_nonexistent),
        "~~Exception 701:\n table tr nonexistent."
        "\n~~Exception in command:\"drop table tr;\"\n");
}

TEST_F(DROP_TABLE_TESTS, TEST_2) {
    CHECK_REQUEST_ST_CLIENT("create table a(b int);", 0, "");
    CHECK_REQUEST_ST_CLIENT("create table b(c real);", 0, "");
    CHECK_REQUEST_ST_CLIENT("drop table a;", 0, "");
    CHECK_REQUEST_ST_CLIENT("create table a(c varchar(100));", 0, "");
    CHECK_REQUEST_ST_CLIENT("show create table b;", 0,
                            "CREATE TABLE b(\n    c real\n);\n");
}

TEST_F(DROP_TABLE_TESTS, TEST_3) {
    CHECK_REQUEST_ST_CLIENT("drop table;",
                            EXCEPTION2NUMB(exc::ExceptionType::syntax),
                            "~~Exception 1:\n wrong syntax!\n"
                            "~~Exception in command:\"drop table;\"\n");
}

class SELECT_TESTS : public REQUEST_TESTS {};

TEST_F(SELECT_TESTS, TEST_1) {
    CHECK_REQUEST_ST_CLIENT("create table a(a int, b real, c varchar(100));", 0,
                            "");
    CHECK_REQUEST_ST_CLIENT("select * from a;", 0, "");
}

TEST_F(SELECT_TESTS, TEST_2) {
    CHECK_REQUEST_ST_CLIENT("create table a(a int, b int, c int);", 0, "");
    CHECK_REQUEST_ST_CLIENT("select *, c, a from a;", 0, "");
}

TEST_F(SELECT_TESTS, TEST_3) {
    CHECK_REQUEST_ST_CLIENT("create table a(a int, b real, c varchar(100));", 0,
                            "");
    CHECK_REQUEST_ST_CLIENT("select c, * from a;",
                            EXCEPTION2NUMB(exc::ExceptionType::syntax),
                            "~~Exception 1:\n wrong syntax!\n"
                            "~~Exception in command:\"select c, * from a;\"\n");
}

TEST_F(SELECT_TESTS, TEST_4) {
    CHECK_REQUEST_ST_CLIENT(
        "select * from a;",
        EXCEPTION2NUMB(exc::ExceptionType::access_table_nonexistent),
        "~~Exception 701:\n table a nonexistent.\n"
        "~~Exception in command:\"select * from a;\"\n");
}

TEST_F(SELECT_TESTS, TEST_5) {
    CHECK_REQUEST_ST_CLIENT("create table a(a int, b real, c varchar(100));", 0,
                            "");
    CHECK_REQUEST_ST_CLIENT(
        "select *, c, a, f from a;",
        exc::ExceptionType ::access_column_nonexistent,
        "~~Exception 702:\n column f in table a nonexistent.\n~~Exception in "
        "command:\"select *, c, a, f from a;\"\n");
}

TEST_F(SELECT_TESTS, TEST_6) {
    CHECK_REQUEST_ST_CLIENT("create table a(a int, b real, c varchar(100));", 0,
                            "");
    CHECK_REQUEST_ST_CLIENT("drop table a;", 0, "");
    CHECK_REQUEST_ST_CLIENT(
        "select * from a;",
        EXCEPTION2NUMB(exc::ExceptionType::access_table_nonexistent),
        "~~Exception 701:\n table a nonexistent.\n"
        "~~Exception in command:\"select * from a;\"\n");
}

TEST_F(SELECT_TESTS, TEST_7) {
    CHECK_REQUEST_ST_CLIENT("create table a(a int, b real, c varchar(100));", 0,
                            "");
    CHECK_REQUEST_ST_CLIENT("insert into a values (1, 0, '1');", 0, "");
    CHECK_REQUEST_ST_CLIENT("insert into a values (1, 1, '0');", 0, "");
    CHECK_REQUEST_ST_CLIENT("insert into a values (0, 1, '1');", 0, "");
    CHECK_REQUEST_ST_CLIENT(
        "select * from a where a = 0;", 0,
        get_select_answer({"a.a", "a.b", "a.c"}, {{"0", "1.000000", "1"}}));
}

TEST_F(SELECT_TESTS, TEST_8) {
    CHECK_REQUEST_ST_CLIENT("create table a(a int);", 0, "");
    CHECK_REQUEST_ST_CLIENT("insert into a values (1);", 0, "");
    CHECK_REQUEST_ST_CLIENT(
        "select * from a where a = '1';",
        EXCEPTION2NUMB(exc::ExceptionType::compare_data_type_mismatch),
        "~~Exception 605:\n can't compare int and varchar.\n~~Exception in "
        "command:\"select * from a where a = '1';\"\n");
}

class INSERT_TESTS : public REQUEST_TESTS {};

TEST_F(INSERT_TESTS, INSERT_TEST_1) {
    CHECK_REQUEST_ST_CLIENT("create table a(a int, b real, c varchar(100));", 0,
                            "");
    CHECK_REQUEST_ST_CLIENT("insert into a values (3, 2.2, 'Hello');", 0, "");
    CHECK_REQUEST_ST_CLIENT(
        "select * from a;", 0,
        get_select_answer({"a.a", "a.b", "a.c"}, {{"3", "2.200000", "Hello"}}));
}

TEST_F(INSERT_TESTS, INSERT_TEST_2) {
    CHECK_REQUEST_ST_CLIENT("create table a(a int, b real, c varchar(100));", 0,
                            "");
    CHECK_REQUEST_ST_CLIENT("insert into a (c, a, b) values ('Hello', 3, 2.2);",
                            0, "");
    CHECK_REQUEST_ST_CLIENT(
        "select * from a;", 0,
        get_select_answer({"a.a", "a.b", "a.c"}, {{"3", "2.200000", "Hello"}}));
}

TEST_F(INSERT_TESTS, INSERT_TEST_3) {
    CHECK_REQUEST_ST_CLIENT("create table a(a int, b real, c varchar(100));", 0,
                            "");
    CHECK_REQUEST_ST_CLIENT(
        "insert into a values (2.2, 3.3, 'Hello');",
        EXCEPTION2NUMB(exc::ExceptionType::set_data_type_mismatch),
        "~~Exception 601:\n value a is not compatible with data type int.\n"
        "~~Exception in command:\"insert into a values (2.2, 3.3, "
        "'Hello');\"\n");
}

TEST_F(INSERT_TESTS, INSERT_TEST_4) {
    CHECK_REQUEST_ST_CLIENT("create table a(a int, b real, c varchar(100));", 0,
                            "");
    CHECK_REQUEST_ST_CLIENT("insert into a values (-2, -1, '');", 0, "");
    CHECK_REQUEST_ST_CLIENT(
        "select * from a;", 0,
        get_select_answer({"a.a", "a.b", "a.c"}, {{"-2", "-1.000000", ""}}));
}

TEST_F(INSERT_TESTS, TEST_5) {
    CHECK_REQUEST_ST_CLIENT("create table a(a int, b real, c varchar(100));", 0,
                            "");
    CHECK_REQUEST_ST_CLIENT(
        "insert into a values (2, 3.3.3, 'Hello');",
        EXCEPTION2NUMB(exc::ExceptionType::syntax),
        "~~Exception 1:\n wrong syntax!\n"
        "~~Exception in command:\"insert into a values (2, 3.3.3, "
        "'Hello');\"\n");
}

TEST_F(INSERT_TESTS, TEST_6) {
    CHECK_REQUEST_ST_CLIENT("create table a(a int, b real, c varchar(100));", 0,
                            "");
    CHECK_REQUEST_ST_CLIENT(
        "insert into a values ('Hello', 3.3, 'Hello');",
        EXCEPTION2NUMB(exc::ExceptionType::set_data_type_mismatch),
        "~~Exception 601:\n value a is not compatible with data type int.\n"
        "~~Exception in command:\"insert into a values ('Hello', 3.3, "
        "'Hello');\"\n");
}

TEST_F(INSERT_TESTS, TEST_7) {
    CHECK_REQUEST_ST_CLIENT("create table a(a int, b real, c varchar(100));", 0,
                            "");
    CHECK_REQUEST_ST_CLIENT(
        "insert into a values (2, 3.3, 4);",
        EXCEPTION2NUMB(exc::ExceptionType::set_data_type_mismatch),
        "~~Exception 601:\n value c is not compatible with data "
        "type varchar.\n~~Exception in command:\"insert into a "
        "values (2, 3.3, 4);\"\n");
}

TEST_F(INSERT_TESTS, TEST_8) {
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

TEST_F(INSERT_TESTS, TEST_9) {
    CHECK_REQUEST_ST_CLIENT(
        "insert into a values (2, 3.3, 'Hello');",
        EXCEPTION2NUMB(exc::ExceptionType::access_table_nonexistent),
        "~~Exception 701:\n table a nonexistent.\n~~Exception in "
        "command:\"insert into a values (2, 3.3, 'Hello');\"\n");
}

TEST_F(INSERT_TESTS, TEST_10) {
    CHECK_REQUEST_ST_CLIENT("create table a(a int, b real, c varchar(100));", 0,
                            "");
    CHECK_REQUEST_ST_CLIENT(
        "insert into a(a, b, f) values (2, 3.3, 'Hello');",
        EXCEPTION2NUMB(exc::ExceptionType::access_column_nonexistent),
        "~~Exception 702:\n column f in table a nonexistent.\n"
        "~~Exception in command:\"insert into a(a, b, f) values (2, 3.3, "
        "'Hello');\"\n");
}

TEST_F(INSERT_TESTS, TEST_11) {
    CHECK_REQUEST_ST_CLIENT("create table a(a int, b real, c varchar(100));", 0,
                            "");
    CHECK_REQUEST_ST_CLIENT("insert into a(a) values (-2);", 0, "");
    CHECK_REQUEST_ST_CLIENT(
        "select * from a;", 0,
        get_select_answer({"a.a", "a.b", "a.c"}, {{"-2", "null", ""}}));
}

TEST_F(INSERT_TESTS, TEST_12) {
    CHECK_REQUEST_ST_CLIENT("create table a(a int, b real, c varchar(100));", 0,
                            "");
    CHECK_REQUEST_ST_CLIENT("insert into a(a) values (8);", 0, "");
    CHECK_REQUEST_ST_CLIENT(
        "select * from a;", 0,
        get_select_answer({"a.a", "a.b", "a.c"}, {{"8", "null", ""}}));
}

TEST_F(INSERT_TESTS, TEST_13) {
    CHECK_REQUEST_ST_CLIENT(
        "create table a(a int not null, b real primary key, c varchar(100) "
        "unique);",
        0, "");
    CHECK_REQUEST_ST_CLIENT("insert into a values (-2, 0.1, 'Hello world!');",
                            0, "");
    CHECK_REQUEST_ST_CLIENT(
        "select * from a;", 0,
        get_select_answer({"a.a", "a.b", "a.c"},
                          {{"-2", "0.100000", "Hello world!"}}));
}

TEST_F(INSERT_TESTS, TEST_14) {
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

TEST_F(INSERT_TESTS, TEST_15) {
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

TEST_F(INSERT_TESTS, TEST_16) {
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

TEST_F(INSERT_TESTS, TEST_17) {
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

TEST_F(INSERT_TESTS, TEST_18) {
    CHECK_REQUEST_ST_CLIENT(
        "create table a(a int not null, b real primary key, c "
        "varchar(100) unique);",
        0, "");
    CHECK_REQUEST_ST_CLIENT("insert into a values (1, 0, 'H!');", 0, "");
    CHECK_REQUEST_ST_CLIENT(
        "insert into a(a, b) values (12, 1);"
        "select * from a;",
        0,
        get_select_answer({"a.a", "a.b", "a.c"},
                          {{"1", "0.000000", "H!"}, {"12", "1.000000", ""}}));
}

TEST_F(INSERT_TESTS, TEST_19) {
    CHECK_REQUEST_ST_CLIENT("create table a(a int, b real, c varchar(100));", 0,
                            "");
    CHECK_REQUEST_ST_CLIENT("insert into a(a, a) values (12, 1);",
                            EXCEPTION2NUMB(exc::ExceptionType::repeat_column),
                            "~~Exception 6:\n repeat column a.\n~~Exception in "
                            "command:\"insert "
                            "into a(a, a) values (12, 1);\"\n");
}

TEST_F(INSERT_TESTS, TEST_20) {
    CHECK_REQUEST_ST_CLIENT("create table a(a int, b real, c varchar(100));", 0,
                            "");
    CHECK_REQUEST_ST_CLIENT(
        "insert into a(a) values (12, 1);",
        EXCEPTION2NUMB(exc::ExceptionType::insert_constants_more_columns),
        "~~Exception 1101:\n the number of constants is more than "
        "columns.\n"
        "~~Exception in command:\"insert into a(a) values (12, 1);\"\n");
}

TEST_F(INSERT_TESTS, TEST_21) {
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

class DELETE_TESTS : public REQUEST_TESTS {};

TEST_F(DELETE_TESTS, TEST_1) {
    CHECK_REQUEST_ST_CLIENT("create table a(a int, b real, c varchar(100));", 0,
                            "");
    CHECK_REQUEST_ST_CLIENT("insert into a values (1, 0, '1');", 0, "");
    CHECK_REQUEST_ST_CLIENT("insert into a values (1, 1, '0');", 0, "");
    CHECK_REQUEST_ST_CLIENT("insert into a values (0, 1, '1');", 0, "");
    CHECK_REQUEST_ST_CLIENT("delete from a where a = 0;", 0, "");
    CHECK_REQUEST_ST_CLIENT(
        "select * from a;", 0,
        get_select_answer({"a.a", "a.b", "a.c"},
                          {{"1", "0.000000", "1"}, {"1", "1.000000", "0"}}));
    CHECK_REQUEST_ST_CLIENT("delete from a where b = 0;", 0, "");
    CHECK_REQUEST_ST_CLIENT(
        "select * from a;", 0,
        get_select_answer({"a.a", "a.b", "a.c"}, {{"1", "1.000000", "0"}}));
    CHECK_REQUEST_ST_CLIENT("delete from a where c = '0';", 0, "");
    CHECK_REQUEST_ST_CLIENT("select * from a;", 0, "");
}

TEST_F(DELETE_TESTS, TEST_2) {
    CHECK_REQUEST_ST_CLIENT("create table a(a int, b real, c varchar(100));", 0,
                            "");
    CHECK_REQUEST_ST_CLIENT("delete from a where c = '0';", 0, "");
    CHECK_REQUEST_ST_CLIENT("select * from a;", 0, "");
}

TEST_F(DELETE_TESTS, TEST_3) {
    CHECK_REQUEST_ST_CLIENT(
        "delete from a where c = '0';",
        EXCEPTION2NUMB(exc::ExceptionType::access_table_nonexistent),
        "~~Exception 701:\n table a nonexistent.\n~~Exception in "
        "command:\"delete from a where c = '0';\"\n");
}

TEST_F(DELETE_TESTS, TEST_4) {
    CHECK_REQUEST_ST_CLIENT("create table a(a int, b real, c varchar(100));", 0,
                            "");
    CHECK_REQUEST_ST_CLIENT("delete from a where f = '0';", 0, "");
}

TEST_F(DELETE_TESTS, TEST_5) {
    CHECK_REQUEST_ST_CLIENT("create table a(a int, b real, c varchar(100));", 0,
                            "");
    CHECK_REQUEST_ST_CLIENT("delete from a where b = '0';", 0, "");
}

TEST_F(DELETE_TESTS, TEST_6) {
    CHECK_REQUEST_ST_CLIENT("create table a(a int, b real, c varchar(100));", 0,
                            "");
    CHECK_REQUEST_ST_CLIENT("insert into a values (1, 0, '1');", 0, "");
    CHECK_REQUEST_ST_CLIENT("insert into a values (1, 1, '0');", 0, "");
    CHECK_REQUEST_ST_CLIENT("insert into a values (0, 1, '1');", 0, "");
    CHECK_REQUEST_ST_CLIENT("insert into a values (0, 0, '1');", 0, "");
    CHECK_REQUEST_ST_CLIENT("delete from a where b = a or a = 1;", 0, "");
    CHECK_REQUEST_ST_CLIENT("select a from a;", 0,
                            get_select_answer({"a.a"}, {{"0"}}));
}

class UPDATE_TESTS : public REQUEST_TESTS {};

TEST_F(UPDATE_TESTS, TEST_1) {
    CHECK_REQUEST_ST_CLIENT("create table a(a int, b real, c varchar(100));", 0,
                            "");
    CHECK_REQUEST_ST_CLIENT("insert into a values (1, 0, '1');", 0, "");
    CHECK_REQUEST_ST_CLIENT("insert into a values (1, 1, '0');", 0, "");
    CHECK_REQUEST_ST_CLIENT("insert into a values (0, 1, '1');", 0, "");
    CHECK_REQUEST_ST_CLIENT("update a set a = 2;", 0, "");
    CHECK_REQUEST_ST_CLIENT(
        "select * from a;", 0,
        get_select_answer({"a.a", "a.b", "a.c"}, {{"2", "1.000000", "1"},
                                                  {"2", "1.000000", "1"},
                                                  {"2", "1.000000", "1"}}));
    CHECK_REQUEST_ST_CLIENT("update a set b = 3.45, c = 'H';", 0, "");
    CHECK_REQUEST_ST_CLIENT(
        "select * from a;", 0,
        get_select_answer({"a.a", "a.b", "a.c"}, {{"2", "3.450000", "H"},
                                                  {"2", "3.450000", "H"},
                                                  {"2", "3.450000", "H"}}));
}

TEST_F(UPDATE_TESTS, TEST_2) {
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

TEST_F(UPDATE_TESTS, TEST_3) {
    CHECK_REQUEST_ST_CLIENT(
        "update a set f = 2;",
        EXCEPTION2NUMB(exc::ExceptionType::access_table_nonexistent),
        "~~Exception 701:\n table a nonexistent.\n~~Exception in "
        "command:\"update a set f = 2;\"\n");
}

TEST_F(UPDATE_TESTS, TEST_4) {
    CHECK_REQUEST_ST_CLIENT("create table a(a int unique);", 0, "");
    CHECK_REQUEST_ST_CLIENT("insert into a values (1);", 0, "");
    CHECK_REQUEST_ST_CLIENT("insert into a values (2);", 0, "");
    CHECK_REQUEST_ST_CLIENT(
        "update a set a = 2;",
        EXCEPTION2NUMB(exc::ExceptionType::duplicated_unique),
        "~~Exception 804 in table a:\n 2 is not unique is in the column "
        "a.\n~~Exception in command:\"update a set a = 2;\"\n");
}

TEST_F(UPDATE_TESTS, TEST_5) {
    CHECK_REQUEST_ST_CLIENT("create table a(a int not null);", 0, "");
    CHECK_REQUEST_ST_CLIENT("insert into a values (1);", 0, "");
    CHECK_REQUEST_ST_CLIENT("insert into a values (2);", 0, "");
    CHECK_REQUEST_ST_CLIENT(
        "update a set a = null;",
        EXCEPTION2NUMB(exc::ExceptionType::null_not_null),
        "~~Exception 805 in table a:\n a can't contain null "
        "values.\n~~Exception in command:\"update a set a = null;\"\n");
}

TEST_F(UPDATE_TESTS, TEST_6) {
    CHECK_REQUEST_ST_CLIENT("create table a(a int primary key);", 0, "");
    CHECK_REQUEST_ST_CLIENT("insert into a values (1);", 0, "");
    CHECK_REQUEST_ST_CLIENT("insert into a values (2);", 0, "");
    CHECK_REQUEST_ST_CLIENT(
        "update a set a = 2;",
        EXCEPTION2NUMB(exc::ExceptionType::duplicated_unique),
        "~~Exception 804 in table a:\n 2 is not unique is in the column "
        "a.\n~~Exception in command:\"update a set a = 2;\"\n");
}

TEST_F(UPDATE_TESTS, TEST_7) {
    CHECK_REQUEST_ST_CLIENT("create table a(a int primary key);", 0, "");
    CHECK_REQUEST_ST_CLIENT("insert into a values (1);", 0, "");
    CHECK_REQUEST_ST_CLIENT("insert into a values (2);", 0, "");
    CHECK_REQUEST_ST_CLIENT(
        "update a set a = '2';",
        EXCEPTION2NUMB(exc::ExceptionType::set_data_type_mismatch),
        "~~Exception 601:\n value a is not compatible with data type "
        "int.\n~~Exception in command:\"update a set a = '2';\"\n");
}

TEST_F(UPDATE_TESTS, TEST_8) {
    CHECK_UNREQUITED_REQUEST_ST_CLIENT("create table a(a int primary key);");
    CHECK_UNREQUITED_REQUEST_ST_CLIENT("insert into a values (2);");
    CHECK_REQUEST_ST_CLIENT("update a set a = 2 where a = 2;", 0, "");
}

class WHERE_TESTS : public REQUEST_TESTS {};

TEST_F(WHERE_TESTS, TEST_1) {
    CHECK_REQUEST_ST_CLIENT("create table a (a varchar(100), b varchar(100));",
                            0, "");
    CHECK_REQUEST_ST_CLIENT("insert into a values('ab', 'ab');", 0, "");
    CHECK_REQUEST_ST_CLIENT("select * from a where '1';", 0,
                            get_select_answer({"a.a", "a.b"}, {{"ab", "ab"}}));
    CHECK_REQUEST_ST_CLIENT("insert into a values('ab', 'abc');", 0, "");
    CHECK_REQUEST_ST_CLIENT("insert into a values('ab', 'b');", 0, "");
    CHECK_REQUEST_ST_CLIENT("insert into a values('ab', 'bb');", 0, "");
    CHECK_REQUEST_ST_CLIENT("insert into a values('ab', 'a');", 0, "");
    CHECK_REQUEST_ST_CLIENT("insert into a values('ab', 'aa');", 0, "");
    CHECK_REQUEST_ST_CLIENT("insert into a values('ab', '');", 0, "");
    CHECK_REQUEST_ST_CLIENT(
        "select * from a where a < b;", 0,
        get_select_answer({"a.a", "a.b"},
                          {{"ab", "abc"}, {"ab", "b"}, {"ab", "bb"}}));
    CHECK_REQUEST_ST_CLIENT(
        "select * from a where a <= b;", 0,
        get_select_answer(
            {"a.a", "a.b"},
            {{"ab", "ab"}, {"ab", "abc"}, {"ab", "b"}, {"ab", "bb"}}));
    CHECK_REQUEST_ST_CLIENT("select * from a where a = b;", 0,
                            get_select_answer({"a.a", "a.b"}, {{"ab", "ab"}}));
    CHECK_REQUEST_ST_CLIENT(
        "select * from a where a >= b;", 0,
        get_select_answer(
            {"a.a", "a.b"},
            {{"ab", "ab"}, {"ab", "a"}, {"ab", "aa"}, {"ab", ""}}));
    CHECK_REQUEST_ST_CLIENT(
        "select * from a where a > b;", 0,
        get_select_answer({"a.a", "a.b"},
                          {{"ab", "a"}, {"ab", "aa"}, {"ab", ""}}));
    CHECK_REQUEST_ST_CLIENT(
        "select * from a where a >= b and b > 'aa' or b = 'b';", 0,
        get_select_answer({"a.a", "a.b"}, {{"ab", "ab"}, {"ab", "b"}}));
    CHECK_REQUEST_ST_CLIENT(
        "select * from a where a < b and b != 'abc';", 0,
        get_select_answer({"a.a", "a.b"}, {{"ab", "b"}, {"ab", "bb"}}));
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

TEST_F(WHERE_TESTS, TEST_2) {
    CHECK_REQUEST_ST_CLIENT("create table a (a int, b int);", 0, "");
    CHECK_REQUEST_ST_CLIENT("insert into a values(-3, 3);", 0, "");
    CHECK_REQUEST_ST_CLIENT("select * from a where 1;", 0,
                            get_select_answer({"a.a", "a.b"}, {{"-3", "3"}}));
    CHECK_REQUEST_ST_CLIENT("select * from a where 0;", 0, "");
    CHECK_REQUEST_ST_CLIENT("insert into a values(-1, -1);", 0, "");
    CHECK_REQUEST_ST_CLIENT("insert into a values(-1, -10);", 0, "");
    CHECK_REQUEST_ST_CLIENT("insert into a values(-1, 10);", 0, "");
    CHECK_REQUEST_ST_CLIENT(
        "select * from a where a < b;", 0,
        get_select_answer({"a.a", "a.b"}, {{"-3", "3"}, {"-1", "10"}}));
    CHECK_REQUEST_ST_CLIENT(
        "select * from a where a <= b;", 0,
        get_select_answer({"a.a", "a.b"},
                          {{"-3", "3"}, {"-1", "-1"}, {"-1", "10"}}));
    CHECK_REQUEST_ST_CLIENT(
        "select * from a where (12 + (4.5 * 2 / 3) + 2.0 * 2 - a * (-1)) / "
        "2 = "
        "b + a;",
        0, get_select_answer({"a.a", "a.b"}, {{"-1", "10"}}));
    CHECK_REQUEST_ST_CLIENT(
        "select a from a where a + 2 >= -0.567;", 0,
        get_select_answer({"a.a"}, {{"-1"}, {"-1"}, {"-1"}}));
    CHECK_REQUEST_ST_CLIENT(
        "select * from a where b != a and b != 10 and b != -10;", 0,
        get_select_answer({"a.a", "a.b"}, {{"-3", "3"}}));
}

TEST_F(WHERE_TESTS, TEST_3) {
    CHECK_REQUEST_ST_CLIENT("create table a (a real, b real);", 0, "");
    CHECK_REQUEST_ST_CLIENT("insert into a values(-3.25, 3);", 0, "");
    CHECK_REQUEST_ST_CLIENT("select * from a where 0.0;", 0, "");
    CHECK_REQUEST_ST_CLIENT(
        "select * from a where 0.12;", 0,
        get_select_answer({"a.a", "a.b"}, {{"-3.250000", "3.000000"}}));
    CHECK_REQUEST_ST_CLIENT("insert into a values(-1.0, -1);", 0, "");
    CHECK_REQUEST_ST_CLIENT("insert into a values(0.123, -10);", 0, "");
    CHECK_REQUEST_ST_CLIENT(
        "select * from a where a + b < (-1.234 * (-1) + 2.05 * 3 - 1.2 / "
        "2.1) / (-3.7);",
        0,
        get_select_answer({"a.a", "a.b"}, {{"-1.000000", "-1.000000"},
                                           {"0.123000", "-10.000000"}}));
    CHECK_REQUEST_ST_CLIENT(
        "select * from a where a  <= b;", 0,
        get_select_answer({"a.a", "a.b"}, {{"-3.250000", "3.000000"},
                                           {"-1.000000", "-1.000000"}}));
    CHECK_REQUEST_ST_CLIENT(
        "select * from a where a / 3.25 * 3 = b * -1;", 0,
        get_select_answer({"a.a", "a.b"}, {{"-3.250000", "3.000000"}}));
    CHECK_REQUEST_ST_CLIENT(
        "select * from a where a >= b;", 0,
        get_select_answer({"a.a", "a.b"}, {{"-1.000000", "-1.000000"},
                                           {"0.123000", "-10.000000"}}));
    CHECK_REQUEST_ST_CLIENT(
        "select * from a where a > 0;", 0,
        get_select_answer({"a.a", "a.b"}, {{"0.123000", "-10.000000"}}));
}

TEST_F(WHERE_TESTS, TEST_4) {
    CHECK_REQUEST_ST_CLIENT("create table a (a int, b real, c varchar(100));",
                            0, "");
    CHECK_REQUEST_ST_CLIENT("insert into a values(1, 2.35, '67 89');", 0, "");
    CHECK_REQUEST_ST_CLIENT(
        "select * from a where a = b - 1.35;", 0,
        get_select_answer({"a.a", "a.b", "a.c"}, {{"1", "2.350000", "67 89"}}));
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
    CHECK_REQUEST_ST_CLIENT(
        "select * from a where not not a = a;", 0,
        get_select_answer({"a.a", "a.b", "a.c"}, {{"1", "2.350000", "67 89"}}));
    CHECK_REQUEST_ST_CLIENT(
        "select * from a where not a != a;", 0,
        get_select_answer({"a.a", "a.b", "a.c"}, {{"1", "2.350000", "67 89"}}));
    CHECK_REQUEST_ST_CLIENT(
        "select * from a where (a = 1) < 3;", 0,
        get_select_answer({"a.a", "a.b", "a.c"}, {{"1", "2.350000", "67 89"}}));
    CHECK_REQUEST_ST_CLIENT(
        "select * from a where (a = 1) > (3 != 3);", 0,
        get_select_answer({"a.a", "a.b", "a.c"}, {{"1", "2.350000", "67 89"}}));
    CHECK_REQUEST_ST_CLIENT("select * from a where (a = 1) <= (3 != 3);", 0,
                            "");
    CHECK_REQUEST_ST_CLIENT("select * from a where (a = 1) * (3 != 3);", 0, "");
    CHECK_REQUEST_ST_CLIENT(
        "select * from a where (a = 1) + (3 != 3);", 0,
        get_select_answer({"a.a", "a.b", "a.c"}, {{"1", "2.350000", "67 89"}}));
    CHECK_REQUEST_ST_CLIENT(
        "select * from a where (a = 1) / 0;", exc::ExceptionType::div_by_zero,
        "~~Exception 9:\n division by zero '1.000000/0.000000'.\n~~Exception "
        "in command:\"select * from a where (a = 1) / 0;\"\n");
    CHECK_REQUEST_ST_CLIENT(
        "select * from a where (a = 1) - (3 != 3);", 0,
        get_select_answer({"a.a", "a.b", "a.c"}, {{"1", "2.350000", "67 89"}}));
    CHECK_REQUEST_ST_CLIENT("insert into a values(3, 0, '');", 0, "");
    CHECK_REQUEST_ST_CLIENT("select * from a where c =;",
                            exc::ExceptionType::syntax,
                            "~~Exception 1:\n wrong syntax!\n~~Exception in "
                            "command:\"select * from a where c =;\"\n");
    CHECK_REQUEST_ST_CLIENT(
        "select * from a where not a < 1 or b < 2 and c = '';", 0,
        get_select_answer({"a.a", "a.b", "a.c"},
                          {{"1", "2.350000", "67 89"}, {"3", "0.000000", ""}}));
    CHECK_REQUEST_ST_CLIENT(
        "select * from a where not a < 1 or b > 2 and c = '';", 0,
        get_select_answer({"a.a", "a.b", "a.c"},
                          {{"1", "2.350000", "67 89"}, {"3", "0.000000", ""}}));
    CHECK_REQUEST_ST_CLIENT(
        "select * from a where a >= 1 and (b = 2 or c = '');", 0,
        get_select_answer({"a.a", "a.b", "a.c"}, {{"3", "0.000000", ""}}));
    CHECK_REQUEST_ST_CLIENT(
        "select * from a where a - 3 = b = 0;", 0,
        get_select_answer({"a.a", "a.b", "a.c"}, {{"1", "2.350000", "67 89"}}));
    CHECK_REQUEST_ST_CLIENT(
        "select * from a where (a - 3 = 0) * 3 = 0;", 0,
        get_select_answer({"a.a", "a.b", "a.c"}, {{"1", "2.350000", "67 89"}}));
    CHECK_REQUEST_ST_CLIENT(
        "select * from a where a - 3 = b * 1 = 0;", 0,
        get_select_answer({"a.a", "a.b", "a.c"}, {{"1", "2.350000", "67 89"}}));
}

class JOIN_TESTS : public REQUEST_TESTS {
   public:
    void SetUp() override {
        CHECK_UNREQUITED_REQUEST_ST_CLIENT(
            "create table a(a int, b varchar(100));"
            "insert into a values(1, 'Viktor');\n"
            "insert into a values(1, 'Danila');\n"
            "insert into a values(0, 'Danila');\n"
            "insert into a values(3, 'Ivan');");
        CHECK_UNREQUITED_REQUEST_ST_CLIENT(
            "create table b(a int, b real);\n"
            "insert into b values(0, 2);\n"
            "insert into b values(3, 0.2);");
        CHECK_UNREQUITED_REQUEST_ST_CLIENT(
            "create table c(a varchar(100), b varchar(100));\n"
            "insert into c values('Danila', 'Write this test.');\n"
            "insert into c values('Ivan', 'Save this data.');\n"
            "insert into c values('Viktor', 'Parsed this request.');\n"
            "insert into c values('Vadik', 'Do nothing.');");
    }
};

TEST_F(JOIN_TESTS, INNER_TEST_1) {
    CHECK_REQUEST_ST_CLIENT(
        "select a.a, b.b, a.b from a INNER JOIN b on a.a = b.a;", 0,
        get_select_answer(
            {"a.a", "b.b", "a.b"},
            {{"0", to_string(2), "Danila"}, {"3", to_string(0.2), "Ivan"}}));
    CHECK_REQUEST_ST_CLIENT(
        "select * from c INNER JOIN a on c.a = a.b;", 0,
        get_select_answer({"a.a", "a.b", "c.a", "c.b"},
                          {
                              {"1", "Viktor", "Viktor", "Parsed this request."},
                              {"1", "Danila", "Danila", "Write this test."},
                              {"0", "Danila", "Danila", "Write this test."},
                              {"3", "Ivan", "Ivan", "Save this data."},
                          }));
    CHECK_REQUEST_ST_CLIENT(
        "select a.a, b.a from a JOIN b on a.a <= b.a;", 0,
        get_select_answer(
            {"a.a", "b.a"},
            {{"1", "3"}, {"1", "3"}, {"0", "0"}, {"0", "3"}, {"3", "3"}}));
}

TEST_F(JOIN_TESTS, FULL_TEST_1) {
    CHECK_REQUEST_ST_CLIENT(
        "select a.b, c.b from c FULL JOIN a on a.b = c.a;", 0,
        get_select_answer({"a.b", "c.b"}, {{"Danila", "Write this test."},
                                           {"Danila", "Write this test."},
                                           {"Ivan", "Save this data."},
                                           {"Viktor", "Parsed this request."},
                                           {"", "Do nothing."}}));
    CHECK_UNREQUITED_REQUEST_ST_CLIENT("insert into b values(5, 20);");
    CHECK_REQUEST_ST_CLIENT(
        "select a.b, b.b from a FULL JOIN b on a.a = b.a;", 0,
        get_select_answer({"a.b", "b.b"}, {{"Danila", to_string(2)},
                                           {"Ivan", to_string(0.2)},
                                           {"Viktor", "null"},
                                           {"Danila", "null"},
                                           {"", to_string(20)}}));
    CHECK_REQUEST_ST_CLIENT(
        "select a.b, b.b from b FULL JOIN a on a.a = b.a;", 0,
        get_select_answer({"a.b", "b.b"}, {{"Danila", to_string(2)},
                                           {"Ivan", to_string(0.2)},
                                           {"", to_string(20)},
                                           {"Viktor", "null"},
                                           {"Danila", "null"}}));
}

TEST_F(JOIN_TESTS, LEFT_TEST_1) {
    CHECK_REQUEST_ST_CLIENT(
        "select a.a, b.b from a LEFT JOIN b on a.a = b.a;", 0,
        get_select_answer({"a.a", "b.b"}, {{"0", to_string(2.0)},
                                           {"3", to_string(0.2)},
                                           {"1", "null"},
                                           {"1", "null"}}));
    CHECK_REQUEST_ST_CLIENT(
        "select a.a, b.b from b LEFT JOIN a on a.a = b.a;", 0,
        get_select_answer({"a.a", "b.b"},
                          {{"0", to_string(2.0)}, {"3", to_string(0.2)}}));
}

TEST_F(JOIN_TESTS, RIGHT_TEST_1) {
    CHECK_REQUEST_ST_CLIENT(
        "select a.a, b.b from a RIGHT JOIN b on a.a = b.a;", 0,
        get_select_answer({"a.a", "b.b"},
                          {{"0", to_string(2.0)}, {"3", to_string(0.2)}}));
    CHECK_REQUEST_ST_CLIENT(
        "select a.a, b.b from b RIGHT JOIN a on a.a = b.a;", 0,
        get_select_answer({"a.a", "b.b"}, {{"0", to_string(2.0)},
                                           {"3", to_string(0.2)},
                                           {"1", "null"},
                                           {"1", "null"}}));
}

TEST_F(JOIN_TESTS, TEST_1) {
    CHECK_REQUEST_ST_CLIENT(
        "select k.b.b, c.b from (a join b on a.a = b.a) as k join c on k.a.b = "
        "c.a;",
        0,
        get_select_answer({"k.b.b", "c.b"},
                          {{to_string(2.0), "Write this test."},
                           {to_string(0.2), "Save this data."}}));
    CHECK_UNREQUITED_REQUEST_ST_CLIENT("create table e(a int);");
    CHECK_REQUEST_ST_CLIENT("select * from e join a on a.a = e.a;", 0, "");
    CHECK_UNREQUITED_REQUEST_ST_CLIENT("insert into e values(5);");
    CHECK_REQUEST_ST_CLIENT("select * from e join a on a.a = e.a;", 0, "");
    CHECK_REQUEST_ST_CLIENT("select a.a, f.b from a INNER JOIN f on a.a = f.a;",
                            exc::ExceptionType ::access_table_nonexistent,
                            "~~Exception 701:\n table f "
                            "nonexistent.\n~~Exception in "
                            "command:\"select a.a, f.b from a INNER JOIN f on "
                            "a.a = f.a;\"\n");
    CHECK_REQUEST_ST_CLIENT(
        "select a.a, b.b from a INNER JOIN b on a.a = b.add;",
        exc::ExceptionType ::access_column_nonexistent,
        "~~Exception 702:\n column add in table b nonexistent.\n~~Exception in "
        "command:\"select a.a, b.b from a INNER JOIN b on a.a = b.add;\"\n");
    CHECK_REQUEST_ST_CLIENT("select a.a, b.b from a INNER JOIN b on a.b = b.a;",
                            exc::ExceptionType::compare_data_type_mismatch,
                            "~~Exception 605:\n can't compare varchar and "
                            "int.\n~~Exception in command:\"select a.a, b.b "
                            "from a INNER JOIN b on a.b = b.a;\"\n");
    CHECK_REQUEST_ST_CLIENT(
        "select t.a.b, b.b, t.c.b from (a join c on a.b = c.a) as t LEFT JOIN "
        "b on t.a.a "
        "= b.a;",
        0,
        get_select_answer({"t.a.b", "b.b", "t.c.b"},
                          {{"Danila", to_string(2.0), "Write this test."},
                           {"Ivan", to_string(0.2), "Save this data."},
                           {"Danila", "null", "Write this test."},
                           {"Viktor", "null", "Parsed this request."}}));
    /*    CHECK_REQUEST_ST_CLIENT(
            "select t.a.a, b.b, t.c.b from (a join c on a.b = c.a) as t LEFT
       JOIN " "b on t.a.a "
            "= b.a;",
            0, "");*/  // TODO: фиг его знает что выдаст.
    CHECK_UNREQUITED_REQUEST_ST_CLIENT(
        "create table a_filter(a int);"
        "insert into a_filter values(1);"
        "insert into a_filter values(0);");
    CHECK_UNREQUITED_REQUEST_ST_CLIENT(
        "create table b_filter(a int);"
        "insert into b_filter values(3);"
        "insert into b_filter values(0);");
    CHECK_REQUEST_ST_CLIENT(
        "select f.g.a.b, t.b.b, f.c.b from (b JOIN b_filter on b.a = "
        "b_filter.a) as t JOIN "
        "((a join a_filter on a.a = a_filter.a) as g JOIN c on g.a.b "
        "= c.a) as f on f.g.a.a = t.b.a;",
        0,
        get_select_answer({"f.g.a.b", "t.b.b", "f.c.b"},
                          {{"Danila", to_string(2.0), "Write this test."}}));
    CHECK_UNREQUITED_REQUEST_ST_CLIENT("insert into b_filter values(-1);");
    CHECK_REQUEST_ST_CLIENT(
        "select * from a_filter join b_filter on  a_filter.a != b_filter.a;", 0,
        get_select_answer(
            {"a_filter.a", "b_filter.a"},
            {{"1", "3"}, {"1", "0"}, {"1", "-1"}, {"0", "3"}, {"0", "-1"}}));
    CHECK_REQUEST_ST_CLIENT(
        "select * from a_filter join b_filter on  a_filter.a <= b_filter.a;", 0,
        get_select_answer({"a_filter.a", "b_filter.a"},
                          {{"1", "3"}, {"0", "3"}, {"0", "0"}}));
    CHECK_REQUEST_ST_CLIENT(
        "select * from a_filter join b_filter on  a_filter.a >= b_filter.a;", 0,
        get_select_answer({"a_filter.a", "b_filter.a"},
                          {{"1", "0"}, {"1", "-1"}, {"0", "0"}, {"0", "-1"}}));
}

TEST_F(JOIN_TESTS, AS_TEST_1) {
    CHECK_REQUEST_ST_CLIENT(
        "select Second.a.a, Second.b.a from (a INNER JOIN b on a.a >= b.a) as "
        "Second;",
        0,
        get_select_answer(
            {"Second.a.a", "Second.b.a"},
            {{"1", "0"}, {"1", "0"}, {"0", "0"}, {"3", "0"}, {"3", "3"}}));
    CHECK_REQUEST_ST_CLIENT(
        "select Second.a.a, Second.a.b from (a INNER JOIN a on a.a = a.a) as "
        "Second;",
        exc::ExceptionType::ambiguous_column_name,
        "~~Exception 7:\n ambiguous column name a.a\n~~Exception in "
        "command:\"select Second.a.a, Second.a.b from (a INNER JOIN a on a.a = "
        "a.a) as Second;\"\n");
    CHECK_REQUEST_ST_CLIENT(
        "select a.a, t.a.b from a INNER JOIN a as t on a.a = t.a.a;",
        exc::ExceptionType::access_column_nonexistent,
        "~~Exception 702:\n column a.a in table t nonexistent.\n~~Exception in "
        "command:\"select a.a, t.a.b from a INNER JOIN a as t on a.a = "
        "t.a.a;\"\n");
    /*    CHECK_REQUEST_ST_CLIENT(
            "select Second.a.a, Second.b.b from (a as b INNER JOIN a on a.a = "
            "b.a) as Second;",
            0, //TODO: данила. тут явно мало записей выводит, я посмотрел,
       реально больше должно быть get_select_answer({"Second.a.a",
       "Second.b.b"}, {{"1", "Viktor"},
                                                               {"1", "Danila"},
                                                               {"0", "Danila"},
                                                               {"3",
       "Ivan"}}));*/
    CHECK_REQUEST_ST_CLIENT(
        "select Second.b.a.a, Second.b.a.b from (a as b INNER JOIN a as b on "
        "b.a.a = b.a.a) as Second;",
        exc::ExceptionType::access_column_nonexistent,
        "~~Exception 702:\n column a.a in table b nonexistent.\n~~Exception in "
        "command:\"select Second.b.a.a, Second.b.a.b from (a as b INNER JOIN a "
        "as b on b.a.a = b.a.a) as Second;\"\n");
    CHECK_REQUEST_ST_CLIENT(
        "select Second.a.a, Second.b.a from (a as c INNER JOIN b on c.a >= "
        "b.a) as Second;",
        exc::ExceptionType::access_column_nonexistent,
        "~~Exception 702:\n column a.a in table Second "
        "nonexistent.\n~~Exception in command:\"select Second.a.a, Second.b.a "
        "from (a as c INNER JOIN b on c.a >= b.a) as Second;\"\n");
    CHECK_REQUEST_ST_CLIENT(
        "select M.a.b, b.b, M.c.b from b JOIN (a join c on a.b = c.a) as M on "
        "M.a.a = b.a;",
        0,
        get_select_answer({"M.a.b", "b.b", "M.c.b"},
                          {{"Danila", to_string(2.0), "Write this test."},
                           {"Ivan", to_string(0.2), "Save this data."}}));
}

class UNION_TESTS : public REQUEST_TESTS {
   public:
    void SetUp() override {
        CHECK_UNREQUITED_REQUEST_ST_CLIENT(
            "create table a (a int not null, b varchar(100) not null);\n"
            "insert into a values(0, 'Vitia');\n"
            "insert into a values(0, 'Viktor');\n"
            "insert into a values(1, 'Viktor');\n"
            "insert into a values(1, 'Vitichka');");
        CHECK_UNREQUITED_REQUEST_ST_CLIENT(
            "create table b (a int not null, b varchar(100) not null);\n"
            "insert into b values(0, 'Lera');\n"
            "insert into b values(1, 'Valeria');\n"
            "insert into b values(2, 'Lerochka');");
    }
};

TEST_F(UNION_TESTS, TEST_1) {
    CHECK_REQUEST_ST_CLIENT("select * from a union b;", 0,  // Да, именно так
                            get_select_answer({"a", "b"}, {{"0", "Vitia"},
                                                           {"0", "Viktor"},
                                                           {"1", "Viktor"},
                                                           {"1", "Vitichka"},
                                                           {"0", "Lera"},
                                                           {"1", "Valeria"},
                                                           {"2", "Lerochka"}}));
    CHECK_UNREQUITED_REQUEST_ST_CLIENT(
        "create table a_a (a int not null , b varchar(100) not null);");
    CHECK_REQUEST_ST_CLIENT("select * from a_a union a;", 0,
                            get_select_answer({"a", "b"}, {{"0", "Vitia"},
                                                           {"0", "Viktor"},
                                                           {"1", "Viktor"},
                                                           {"1", "Vitichka"}}));
    CHECK_UNREQUITED_REQUEST_ST_CLIENT("insert into a_a values(100, 'Poni');");
    CHECK_REQUEST_ST_CLIENT("select * from a union b union a_a;", 0,
                            get_select_answer({"a", "b"}, {{"0", "Vitia"},
                                                           {"0", "Viktor"},
                                                           {"1", "Viktor"},
                                                           {"1", "Vitichka"},
                                                           {"0", "Lera"},
                                                           {"1", "Valeria"},
                                                           {"2", "Lerochka"},
                                                           {"100", "Poni"}}));
    CHECK_REQUEST_ST_CLIENT("select * from (a union b) as u union a_a;", 0,
                            get_select_answer({"a", "b"}, {{"0", "Vitia"},
                                                           {"0", "Viktor"},
                                                           {"1", "Viktor"},
                                                           {"1", "Vitichka"},
                                                           {"0", "Lera"},
                                                           {"1", "Valeria"},
                                                           {"2", "Lerochka"},
                                                           {"100", "Poni"}}));
    CHECK_UNREQUITED_REQUEST_ST_CLIENT("insert into b values(0, 'Vitia');");
    CHECK_REQUEST_ST_CLIENT("select * from a union b;", 0,
                            get_select_answer({"a", "b"}, {{"0", "Vitia"},
                                                           {"0", "Viktor"},
                                                           {"1", "Viktor"},
                                                           {"1", "Vitichka"},
                                                           {"0", "Lera"},
                                                           {"1", "Valeria"},
                                                           {"2", "Lerochka"}}));
}

TEST_F(UNION_TESTS, EXCEPTION_TEST_1) {
    /*    CHECK_REQUEST_ST_CLIENT(
            "select * from a union select * from b;", -1,
            ""); */ // Это даёт ошибку, т. к. вложенные селекты ещё не задовали.
    CHECK_REQUEST_ST_CLIENT(
        "select * from a union f;",
        exc::ExceptionType::access_table_nonexistent,
        "~~Exception 701:\n table f nonexistent.\n~~Exception in "
        "command:\"select * from a union f;\"\n");  // TODO: не существует
    /*    CHECK_REQUEST_ST_CLIENT(
            "select * from a union a;", -1,
            ""); */ // TODO: если это риализованно и нормально работает, то перемести
    CHECK_UNREQUITED_REQUEST_ST_CLIENT(
        "create table a_a (a int not null, b varchar(100));");
    CHECK_REQUEST_ST_CLIENT(
        "select * from a_a union a;", exc::ExceptionType::null_column_in_union,
        "~~Exception 1104:\n Union requires all columns to be not "
        "null.\n~~Exception in command:\"select * from a_a union a;\"\n");
    CHECK_UNREQUITED_REQUEST_ST_CLIENT(
        "create table a_b (a int, b varchar(100) primary key);");
    CHECK_REQUEST_ST_CLIENT("select * from a union a_b;", -1, "");
    CHECK_UNREQUITED_REQUEST_ST_CLIENT(
        "create table long_a (a int not null, b varchar(100) not null, c varchar(100) not null);");
    CHECK_REQUEST_ST_CLIENT("a union long_a;", -1,
                            "");  // TODO: не совпадает количество колонок
}

class INTERSECT_TESTS : public REQUEST_TESTS {
   public:
    void SetUp() override {
        CHECK_UNREQUITED_REQUEST_ST_CLIENT(
            "create table a (a int);\n"
            "insert into a values(0);\n"
            "insert into a values(0);\n"
            "insert into a values(1);\n"
            "insert into a values(4);");
        CHECK_UNREQUITED_REQUEST_ST_CLIENT(
            "create table b (a int,);\n"
            "insert into b values(1);\n"
            "insert into b values(1);\n"
            "insert into b values(2);");
    }
};

TEST_F(INTERSECT_TESTS, TEST_1) {
    CHECK_REQUEST_ST_CLIENT("a intersect b;", 0,
                            get_select_answer({"a"}, {{"0"}, {"1"}}));
    CHECK_UNREQUITED_REQUEST_ST_CLIENT("create table a_a (a int);");
    CHECK_REQUEST_ST_CLIENT("a_a intersect b;", 0, "");
    CHECK_UNREQUITED_REQUEST_ST_CLIENT(
        "create table a_a (a unique int); insert into a_a values(2);");
    CHECK_REQUEST_ST_CLIENT("a_a intersect a;", 0,
                            get_select_answer({"a"}, {{"2"}}));
    CHECK_UNREQUITED_REQUEST_ST_CLIENT("insert into a_a values(0);");
    CHECK_REQUEST_ST_CLIENT("a_a intersect a intersect b;", 0,
                            get_select_answer({"a"}, {{"0"}}));
    CHECK_REQUEST_ST_CLIENT("(a_a intersect a) intersect (b);", 0,
                            get_select_answer({"a"}, {{"0"}}));
}

TEST_F(INTERSECT_TESTS, EXCEPTION_TEST_1) {
    CHECK_REQUEST_ST_CLIENT("select * from a intersect select * from b;", -1,
                            "");
    CHECK_REQUEST_ST_CLIENT("a_a intersect b;",
                            exc::ExceptionType::access_table_nonexistent, "");
    CHECK_REQUEST_ST_CLIENT("b intersect b;", -1, "0");
    CHECK_UNREQUITED_REQUEST_ST_CLIENT(
        "create table long_a (a int, b varchar(100), c varchar(100));");
    CHECK_REQUEST_ST_CLIENT("a intersect long_a;", -1,
                            "");  // TODO: не совпадает количество колонок
}

class DROP_TESTS : public ::testing::Test {
   public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void TearDown() override { clearDB(); }

    static ourSQL::client::Client client;
};

void DROP_TESTS::SetUpTestCase() {
    clearDB();
#if defined(CREATE_SERVER)
    Server::get()->run();
#endif
    client.connect();
}
void DROP_TESTS::TearDownTestCase() {
#if defined(CREATE_SERVER)
    Server::get()->stop();
#endif
}

ourSQL::client::Client DROP_TESTS::client(TEST_SERVER_HOST, TEST_SERVER_PORT);

TEST_F(DROP_TESTS, CREATE_TEST_1) {
    CHECK_DROP_REQUEST_ST_CLIENT(
        "create table a (b int);", "show create table a;", 0,
        "CREATE TABLE a(\n    b int\n);\n", "a a_meta");
}

TEST_F(DROP_TESTS, DROP_TEST_1) {
    CHECK_UNREQUITED_REQUEST_ST_CLIENT("create table a (b int);");
    CHECK_DROP_REQUEST_ST_CLIENT(
        "drop table a;", "show create table a;",
        EXCEPTION2NUMB(exc::ExceptionType::access_table_nonexistent),
        "\n~~Exception 701:\n table a nonexistent.\n~~Exception in "
        "command:\"show create table a;\"\n",
        "a a_meta");
}

TEST_F(DROP_TESTS, INSERT_TEST_1) {
    CHECK_UNREQUITED_REQUEST_ST_CLIENT(
        "create table a(a int, b real, c varchar(100));");
    CHECK_UNREQUITED_REQUEST_ST_CLIENT("insert into a values (1, 0, '1');");
    CHECK_DROP_REQUEST_ST_CLIENT(
        "insert into a values (1, 0, '1');", "select * from a;", 0,
        get_select_answer({"a", "b", "c"}, {{"1", to_string(0.0), "1"}}),
        "a a_meta");
}

TEST_F(DROP_TESTS, DELETE_TEST_1) {
    CHECK_UNREQUITED_REQUEST_ST_CLIENT(
        "create table a(a int, b real, c varchar(100));");
    CHECK_UNREQUITED_REQUEST_ST_CLIENT("insert into a values (1, 1, '1');");
    CHECK_UNREQUITED_REQUEST_ST_CLIENT("insert into a values (2, 2, '2');");
    CHECK_DROP_REQUEST_ST_CLIENT(
        "delete from a where a = 2", "select * from a;", 0,
        get_select_answer({"a", "b", "c"}, {{"1", to_string(1.0), "1"}}),
        "a a_meta");
}

TEST_F(DROP_TESTS, UPDATE_TEST_1) {
    CHECK_UNREQUITED_REQUEST_ST_CLIENT(
        "create table a(a int, b real, c varchar(100));");
    CHECK_UNREQUITED_REQUEST_ST_CLIENT("insert into a values (2, 2, '2');");
    CHECK_DROP_REQUEST_ST_CLIENT(
        "update a set a = 1, b = 1, c = '1';", "select * from a;", 0,
        get_select_answer({"a", "b", "c"}, {{"1", to_string(1.0), "1"}}),
        "a a_meta");
}