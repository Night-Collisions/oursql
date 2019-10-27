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

    std::string name = TEST_DIRECTORY_DB;

    command = delete_command + name;
    std::system(command.c_str());

    command = create_dir_command + name;
    std::system(command.c_str());
}

void Server::run() {
    stop();
    std::string command;
#if defined(WIN32) || defined(_WIN32) || \
    defined(__WIN32) && !defined(__CYGWIN__)
    command = "start cmd.exe /c \"" + directory_ + name_ + ".exe\"";
#else
    assert(0);
#endif
    std::system(command.c_str());
}

void Server::stop() {
    std::string command;
#if defined(WIN32) || defined(_WIN32) || \
    defined(__WIN32) && !defined(__CYGWIN__)
    command = "taskkill /im " + name_ + ".exe /f";
#else
    assert(0);
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

void test_sleep(size_t time) {
#if defined(WIN32) || defined(_WIN32) || \
    defined(__WIN32) && !defined(__CYGWIN__)
    Sleep(time);
#else
    usleep(time);
#endif
}

void coppyDB() {
    std::string delete_command;
    std::string copy_command;
    const std::string copy_dir = std::string(TEST_DIRECTORY_DB) + "_copy";
#if defined(WIN32) || defined(_WIN32) || \
    defined(__WIN32) && !defined(__CYGWIN__)
    delete_command = std::string("rmdir /Q /S ") + copy_dir;
    copy_command =
        std::string("xcopy /E /I /Q ") + TEST_DIRECTORY_DB + " " + copy_dir;
#else
    assert(0);
#endif

    std::system(delete_command.c_str());
    std::system(copy_command.c_str());
}

bool compareFile(const std::string& name1, const std::string& name2) {
    std::ifstream f1(name1);
    std::ifstream f2(name2);
    if (!f1.is_open() || !f2.is_open()) {
        return !f1.is_open() && !f2.is_open();
    }
    while (!f1.eof() && !f2.eof()) {
        if (f1.get() != f2.get()) {
            return false;
        }
    }
    return f1.eof() && f2.eof();
}

std::vector<std::string> split_string(const std::string& s) {
    std::stringstream str(s);
    std::vector<std::string> ans;
    while (!str.eof()) {
        std::string buff;
        str >> buff;
        ans.push_back(buff);
    }
    return ans;
}

std::string drop_test(const std::string& request,
                      const std::string& checker_request,
                      const long checker_exception,
                      const std::string& checker_answer,
                      const std::string& db_files, Client& client,
                      size_t start_time, size_t step_time, size_t max_time) {
    client.setExceptionReconnect(false);
    coppyDB();
    auto files = split_string(db_files);
    for (unsigned int count = 0; 1; count++) {
        if (count * step_time >= max_time) {
            return "Time out.";
        }
        Server::get()->run();
        client.connect();
        std::string ans;
        client.sendRequest(request);
        test_sleep(start_time + count * step_time);
        Server::get()->stop();
        Server::get()->run();
        client.connect();
        if (check_request(checker_request, checker_exception, checker_answer, client).empty()) {
            return "";
        }
        for (const auto& j : files)
            if (!compareFile(std::string(TEST_DIRECTORY_DB) + "\\" + j,
                             std::string(TEST_DIRECTORY_DB) + "_copy\\" + j)) {
                return "Changed file " + j;
            }
    }
}

std::string get_select_answer(
    const std::vector<std::string>& column,
    const std::vector<std::vector<std::string>>& data) {
    std::string ans;
    for (const auto& i : data) {
        assert(column.size() == i.size());
    }
    for (const auto& i : data) {
        ans += "=======\n";
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
