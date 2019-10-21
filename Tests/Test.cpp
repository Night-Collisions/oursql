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
    
    std::string name = "DataBD";
    
    command = delete_command + name;
    std::system(command.c_str());

    command = create_dir_command + name;
    std::system(command.c_str());
}

void run_server() {
    std::string command;
    #if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
        command = "start cmd.exe /c \"..\\Server\\OurSQL_Server.exe\"";
    #else
    #endif
    std::system(command.c_str());
}

void stop_server() {
    std::string command;
    #if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
        command = "taskkill /im OurSQL_Server.exe /f";
    #else
    #endif
    std::system(command.c_str());
}
