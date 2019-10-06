#include "Test.h"

bool operator==(const Column& a, const Column& b) {
    return a.getName() == b.getName() && a.getType() == b.getType() &&
        a.getConstraint() == b.getConstraint();
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

void clearDB() {
    std::string command;
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    const char *delete_command = "rmdir /Q /S ";
    const char *create_dir_command = "md ";
#else
#endif
    
    std::string name = "DataBD";
    
    command = delete_command + name;
    std::system(command.c_str());

    command = create_dir_command + name;
    std::system(command.c_str());
}
