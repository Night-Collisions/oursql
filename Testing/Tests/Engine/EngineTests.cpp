#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../../../App/Engine/Engine.h"
#include "../Test.h"

TEST(Engine_Exists, SimpleTest) {
    std::string name = "Engine_Exists_SimpleTest";
    std::ofstream(getPathToTable(name)).close();
    std::ofstream(getPathToTableMeta(name)).close();
    EXPECT_TRUE(exists(name));
    std::remove(getPathToTable(name).c_str());
    std::remove(getPathToTableMeta(name).c_str());
}

TEST(Engine_Exists, NotExists) {
    std::string name = "Engine_Exists_NotExists";
    EXPECT_FALSE(exists(name));
}

TEST(Engine_Drop, SimpleTest) {
    std::string name = "Engine_Drop_SimpleTest";
    std::ofstream(getPathToTable(name)).close();
    std::ofstream(getPathToTableMeta(name)).close();
    EXPECT_TRUE(exists(name));
    EXPECT_FALSE(drop(name));
    EXPECT_FALSE(exists(name));
    EXPECT_FALSE(static_cast<bool>(std::ifstream(getPathToTable(name).c_str())));
    EXPECT_FALSE(static_cast<bool>(std::ifstream(getPathToTableMeta(name).c_str())));
}

TEST(Engine_Drop, NotExists) {
    std::string name = "Engine_Drop_NotExists";
    ASSERT_FALSE(exists(name));
    EXPECT_TRUE(drop(name));
}

TEST(Engine_Drop, DoubleDrop) {
    std::string name = "Engine_Drop_DoubleDrop";
    std::ofstream(getPathToTable(name)).close();
    std::ofstream(getPathToTableMeta(name)).close();
    ASSERT_FALSE(drop(name));
    EXPECT_TRUE(drop(name));
}

TEST(Engine_Create, SimpleTest) {
    std::string name = "Engine_Create_SimpleTest";
    std::vector<Column> columns = {
            {"column1", DataType::integer},
            {"column2", DataType::real, {
                ColumnConstraint::not_null, ColumnConstraint::unique, ColumnConstraint::primary_key}
            },
            {"column3", DataType::text, {ColumnConstraint::unique}}
    };
    create(Table(name, columns));

    std::ifstream metafile(getPathToTableMeta(name));
    std::stringstream sstream;
    sstream << metafile.rdbuf();
    const char* expected = "{\"name\":\"Engine_Create_SimpleTest\",\"columns\":["
                           "{\"name\":\"column1\",\"type\":0,\"constraints\":[]},"
                           "{\"name\":\"column2\",\"type\":1,\"constraints\":[0,1,2]},"
                           "{\"name\":\"column3\",\"type\":2,\"constraints\":[2]}]}";

    EXPECT_EQ(expected, sstream.str());
    drop(name);
}

TEST(Engine_Create, DoubleCreate) {
    std::string name = "Engine_Create_DoubleCreate";
    std::vector<Column> columns = {
            {"column1", DataType::integer},
            {"column2", DataType::real, {
                ColumnConstraint::not_null, ColumnConstraint::unique, ColumnConstraint::primary_key}
            },
            {"column3", DataType::text, {ColumnConstraint::unique}}
    };
    EXPECT_EQ(create(Table(name, columns)), false);
    EXPECT_EQ(create(Table(name, columns)), true);
    drop(name);
}

TEST(Engine_Show, SimpleTest) {
    std::string name = "Engine_Show_SimpleTest";
    std::vector<Column> columns = {
            {"column1", DataType::integer},
            {"column2", DataType::real, {
                ColumnConstraint::not_null, ColumnConstraint::unique, ColumnConstraint::primary_key}
            },
            {"column3", DataType::text, {ColumnConstraint::unique}}
    };
    Table expected(name, columns);
    create(expected);
    EXPECT_EQ(expected, show(name));
    drop(name);
}

TEST(Engine_Show, NotExists) {
    std::string name = "Engine_Show_NotExists";
    EXPECT_TRUE(show(name).getName().empty());
}

TEST(Engine_ShowCreate, SimpleTest) {
    std::string name = "Engine_ShowCreate_SimpleTest";
    std::vector<Column> columns = {
            {"column1", DataType::integer},
            {"column2", DataType::real, {
                ColumnConstraint::not_null, ColumnConstraint::unique, ColumnConstraint::primary_key}
            },
            {"column3", DataType::text, {ColumnConstraint::unique}}
    };
    create(Table(name, columns));

    const char* expected = "CREATE TABLE Engine_ShowCreate_SimpleTest(\n"
                           "    column1 int,\n"
                           "    column2 real primary key not null unique,\n"
                           "    column3 text unique\n"
                           ");";
    EXPECT_EQ(expected, showCreate(name));
    drop(name);
}

TEST(Engine_ShowCreate, NotExists) {
    std::string name = "Engine_ShowCreate_NotExists";
    EXPECT_TRUE(showCreate(name).empty());
}