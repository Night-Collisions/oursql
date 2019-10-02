#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../../App/Engine/Field.h"
#include "../../App/Engine/Table.h"
#include "../../App/Engine/Engine.h"

class EngineAPI : ::testing::Test {
    void SetUp() override {
        std::cout << "SetUp" << std::endl;
    }
    void TearDown() override {
        std::cout << "TearDown" << std::endl;
    }
};


TEST(EngineAPI, SimpleTest) {
    create({"TableName", {{"ParamName", DataType::integer, {FieldConstraint::not_null}}}});
}