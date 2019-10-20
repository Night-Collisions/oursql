#include "Test.h"

#include "../Server/Engine/Engine.h"
#include "../Server/Our.h"

void clearDB() {
    std::string command;
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    const char *delete_command = "rmdir /Q /S ";
    const char *create_dir_command = "md ";
#else
    const char *delete_command = "rm -rf ";
    const char *create_dir_command = "mkdir ";
#endif
    
    std::string name = "..\\Server\\DataBD";
    
    command = delete_command + name;
    std::system(command.c_str());

    command = create_dir_command + name;
    std::system(command.c_str());

    Engine::freeAll();
}

void check_requests(const std::vector<request_description>& requests, Client& client) {
    for (const auto& i : requests) {
        std::string out;
        ASSERT_EQ(client.request(i.request, out), i.exception) << "Error in request:\n  " + i.request;
        EXPECT_EQ(out, i.answer) << "Error in request:\n  " + i.request;
    }
}
