#include "Test.h"

#include "../Server/Engine/Engine.h"
#include "../Server/Our.h"

bool operator==(const Column& a, const Column& b) {
    return a.getName() == b.getName() && a.getType() == b.getType() &&
        a.getConstraint() == b.getConstraint();
}

bool operator==(const Table& a, const Table& b) {
    auto a_columns = a.getColumns();
    auto b_columns = b.getColumns();
    if (a.getName() != b.getName() || a_columns.size() != b_columns.size()) {
        return false;
    }
    for (const auto& i : a_columns) {
        bool eq = false;
        for (const auto& j : b_columns) {
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

void clearDB() {
    std::string command;
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    const char *delete_command = "rmdir /Q /S ";
    const char *create_dir_command = "md ";
#else
    const char *delete_command = "rm -rf ";
    const char *create_dir_command = "mkdir ";
#endif
    
    std::string name = "DataBD";
    
    command = delete_command + name;
    std::system(command.c_str());

    command = create_dir_command + name;
    std::system(command.c_str());

    Engine::freeAll();
}

void check_requests(const std::vector<request_description>& requests) {
    for (const auto& i : requests) {
        std::stringstream in(i.request);
        std::stringstream out;
        ASSERT_EQ(ourSQL::perform(in, out), i.exception);
        EXPECT_EQ(out.str(), i.answer);
    }
}
