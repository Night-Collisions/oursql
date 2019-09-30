#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../../App/Engine/Field.h"
#include "../../App/Engine/Table.h"
#include "parser.cpp"

extern Table getTable();

bool operator==(const Field& a, const Field& b) {
    return a.getName() == b.getName() && a.getType() == b.getType() &&
           a.getConstraint() == b.getConstraint() && a.getVal() == b.getVal();
}

bool operator==(const Table& a, const Table& b) {
    auto a_fields = a.getFields();
    auto b_fields = b.getFields();
    if (a.getName() != b.getName() || a_fields.size() != b_fields.size()) {
        return false;
    }
    for (const auto& i : a_fields) {
        bool eq = false;
        for (const auto& j : b_fields) {
            if (i == j) {
                eq = true;
                break;
            }
        }
        if (!eq) {
            return false;
        }
    }
    return true;
}

void setParserRequest(const std::string& s) {
    FILE* f;
    f = tmpfile();
    fwrite(s.c_str(), 1, s.size(), f);
    rewind(f);
    yyin = f;
}

TEST(Parser_CreateTable, CreateSimpleTable) {
    setParserRequest("create table a(f int);\n");
    yyparse();
    auto table = getTable();
    Table expect_table("a", {{"f", DataType::integer}});
    EXPECT_EQ(table, expect_table);
}
