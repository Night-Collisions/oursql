#include "Test.h"

#include "../Server/Engine/Engine.h"
#include "../Server/Our.h"

Server* Server::obj_ = nullptr;

void clearDB() {
    std::string command;
#if defined(WIN32) || defined(_WIN32) || \
    defined(__WIN32) && !defined(__CYGWIN__)
    const char* delete_command = "rmdir /Q /S ";
    const char* create_dir_command = "md ";
#else
    const char* delete_command = "rm -rf ";
    const char* create_dir_command = "mkdir ";
#endif

    std::string name = "DataBD";

    command = delete_command + name;
    std::system(command.c_str());

    command = create_dir_command + name;
    std::system(command.c_str());
}

void Server::run() {
    std::string command;
#if defined(WIN32) || defined(_WIN32) || \
    defined(__WIN32) && !defined(__CYGWIN__)
    command = "start cmd.exe /c \"" + directory_ + name_ + ".exe\"";
#else
#endif
    std::system(command.c_str());
}

void Server::stop() {
    std::string command;
#if defined(WIN32) || defined(_WIN32) || \
    defined(__WIN32) && !defined(__CYGWIN__)
    command = "taskkill /im " + name_ + ".exe /f";
#else
#endif
    std::system(command.c_str());
}

std::string check_request(const std::string& request, const long exception,
                          const std::string& answer, Client& client) {
    std::string out;
    const std::string error_message =
        "Error in request:\n    " + ::testing::PrintToString(request) + "\n";
    long exception_request = client.request(request, out);
    if (exception_request != static_cast<long>(exception)) {
        return error_message + "Wrong exception code:\n  expected:\n    " +
               std::to_string(static_cast<long>(exception)) +
               "\n  real:\n    " + std::to_string(exception_request) +
               "\n  Exception message:\n    " + ::testing::PrintToString(out);
    }
    if (out != answer) {
        return error_message + "Wrong exception message:\n  expected:\n    " +
               ::testing::PrintToString(answer) + "\n  real:\n    " +
               ::testing::PrintToString(out);
    }
    return "";
}

std::string get_select_answer(
    const std::vector<std::string>& column,
    const std::vector<std::vector<std::string>>& data) {
    std::string ans;
    for (const auto& i : data) {
        assert(column.size() == i.size());
    }
    for (const auto& i : data) {
        for (unsigned int j = 0; j < i.size(); j++) {
            ans += column[j] + ": " + i[j] + "\n";
        }
    }
    return ans;
}

std::string to_string(double a) {
    std::stringstream stream;
    stream << std::fixed << std::setprecision(6) << a;
    return stream.str();
}
