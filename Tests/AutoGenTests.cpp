#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "Test.h"
#include "parser.cpp"

TEST(AGT, TEST_1) {
    buffer_on = 1;
    parse_string("create table();");
    clearDB();
    EXPECT_EQ(buffer, "syntax error, unexpected LPAREN, expecting ID\nsyntax error, unexpected $end, expecting SEMI\n");
}

TEST(AGT, TEST_2) {
    buffer_on = 1;
    parse_string("create table a(b int);");
    clearDB();
    EXPECT_EQ(buffer, "");
}

TEST(AGT, TEST_3) {
    buffer_on = 1;
    parse_string("create table a(b int not null);");
    clearDB();
    EXPECT_EQ(buffer, "");
}

TEST(AGT, TEST_4) {
    buffer_on = 1;
    parse_string("create table a(b int); show create table a;");
    clearDB();
    EXPECT_EQ(buffer, "CREATE TABLE a(\n    b int\n);");
}

TEST(AGT, TEST_5) {
    buffer_on = 1;
    parse_string("create table a(b int not null not null);");
    clearDB();
    EXPECT_EQ(buffer, "Duplicate constraints\nDuplicate constraints\n");
}

TEST(AGT, TEST_6) {
    buffer_on = 1;
    parse_string("create table a(b int not null, c real unique, d text primary key); show create table a;");
    clearDB();
    EXPECT_EQ(buffer, "CREATE TABLE a(\n    b int not null,\n    c real unique,\n    d text primary key\n);");
}

TEST(AGT, TEST_7) {
    buffer_on = 1;
    parse_string("create table sfs;");
    clearDB();
    EXPECT_EQ(buffer, "syntax error, unexpected SEMI, expecting LPAREN\nsyntax error, unexpected $end, expecting SEMI\n");
}

TEST(AGT, TEST_8) {
    buffer_on = 1;
    parse_string("show ghgh;");
    clearDB();
    EXPECT_EQ(buffer, "syntax error, unexpected ID, expecting CREATE\nsyntax error, unexpected $end, expecting SEMI\n");
}

TEST(AGT, TEST_9) {
    buffer_on = 1;
    parse_string("fdgd");
    clearDB();
    EXPECT_EQ(buffer, "syntax error, unexpected ID, expecting CREATE or SHOW or DROP\n");
}

TEST(AGT, TEST_10) {
    buffer_on = 1;
    parse_string("show create table a;");
    clearDB();
    EXPECT_EQ(buffer, "Table doesn't exist\nTable doesn't exist\n");
}

TEST(AGT, TEST_11) {
    buffer_on = 1;
    parse_string("drop table tr;");
    clearDB();
    EXPECT_EQ(buffer, "Table doesn't exist\nTable doesn't exist\n");
}

TEST(AGT, TEST_12) {
    buffer_on = 1;
    parse_string("create table a(i int); create table b(i int); create table c(i int); show create table a; show create table b; show create table c;");
    clearDB();
    EXPECT_EQ(buffer, "CREATE TABLE c(\n    i int\n);");
}

TEST(AGT, TEST_13) {
    buffer_on = 1;
    parse_string("create table a(b text); create table b(f int); drop table b; show create table a;");
    clearDB();
    EXPECT_EQ(buffer, "CREATE TABLE a(\n    b text\n);");
}

TEST(AGT, TEST_14) {
    buffer_on = 1;
    parse_string("create table a(b text); create table b(f int); drop table b; show create table b;");
    clearDB();
    EXPECT_EQ(buffer, "Table doesn't exist\nTable doesn't exist\n");
}

TEST(AGT, TEST_15) {
    buffer_on = 1;
    parse_string("create table a(b text); drop table a; create table a(b text); show create table a;");
    clearDB();
    EXPECT_EQ(buffer, "CREATE TABLE a(\n    b text\n);");
}

TEST(AGT, TEST_16) {
    buffer_on = 1;
    parse_string("CreaTe     TabLe MyTable(a int); show create table MyTable;");
    clearDB();
    EXPECT_EQ(buffer, "CREATE TABLE MyTable(\n    a int\n);");
}

TEST(AGT, TEST_17) {
    buffer_on = 1;
    parse_string("CreaTe     TabLe MyTable(a int); show create table Mytable;");
    clearDB();
    EXPECT_EQ(buffer, "CREATE TABLE MyTable(\n    a int\n);");
}

TEST(AGT, TEST_18) {
    buffer_on = 1;
    parse_string("create table a(MyColumn int, mycolumn text);");
    clearDB();
    EXPECT_EQ(buffer, "Column 'MyColumn' already exists\nColumn 'MyColumn' already exists\n");
}

TEST(AGT, TEST_19) {
    buffer_on = 1;
    parse_string("drop table;");
    clearDB();
    EXPECT_EQ(buffer, "syntax error, unexpected SEMI, expecting ID\nsyntax error, unexpected $end, expecting SEMI\n");
}

TEST(AGT, TEST_20) {
    buffer_on = 1;
    parse_string("create table a(b text not null unique primary key); show create table a;");
    clearDB();
    EXPECT_EQ(buffer, "CREATE TABLE a(\n    b text primary key not null unique\n);");
}

TEST(AGT, TEST_21) {
    buffer_on = 1;
    parse_string("CREATE TABLE A(C INT NOT NULL); SHOW CREATE TABLE A;");
    clearDB();
    EXPECT_EQ(buffer, "CREATE TABLE A(\n    C int not null\n);");
}

TEST(AGT, TEST_22) {
    buffer_on = 1;
    parse_string("create table a(b text primary key, c int primary key);");
    clearDB();
    EXPECT_EQ(buffer, "Primary key already exists\nPrimary key already exists\n");
}

TEST(AGT, TEST_23) {
    buffer_on = 1;
    parse_string("create table rt(a int);");
    clearDB();
    EXPECT_EQ(buffer, "");
}

