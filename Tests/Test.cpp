#include "Test.h"

#include "../Server/Engine/Engine.h"
#include "../Server/Our.h"

Server* Server::obj_ = nullptr;

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
}

void Server::run() {
    std::string command;
    #if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
        command = "start cmd.exe /c \"" + directory_ + name_ + ".exe\"";
    #else
    #endif
    std::system(command.c_str());
}

void Server::stop() {
    std::string command;
    #if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
        command = "taskkill /im " + name_ + ".exe /f";
    #else
    #endif
    std::system(command.c_str());
}
