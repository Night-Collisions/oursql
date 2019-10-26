#ifndef OURSQL_TEST_H
#define OURSQL_TEST_H

#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../Client/Client.h"

#define CREATE_SERVER

#define TEST_SERVER_HOST ("localhost")
#define TEST_SERVER_PORT (11234)

#define TEST_DIRECTORY_DB ("DataBD")

#define EXCEPTION2NUMB(expect) static_cast<long>(expect)

#define CHECKER_TEST_FUNCTION(funct)  \
    {                            \
        std::string ans = funct; \
        if (!ans.empty()) {      \
            FAIL() << ans;       \
        }                        \
    }

#define CHECK_REQUEST(request_message, exception, answer, client)         \
    {                                                                     \
        CHECKER_TEST_FUNCTION(check_request(                                   \
            request_message, EXCEPTION2NUMB(exception), answer, client)); \
    }

#define CHECK_REQUEST_ST_CLIENT(request_message, exception, answer) \
    { CHECK_REQUEST(request_message, exception, answer, client); }

#define CHECK_UNREQUITED_REQUEST(request_message, client) \
    { CHECK_REQUEST(request_message, 0, "", client); }

#define CHECK_UNREQUITED_REQUEST_ST_CLIENT(request_message) \
    CHECK_UNREQUITED_REQUEST(request_message, client)

#define CHECK_DROP_RECUEST(request, checker_request, checker_exception, \
                                checker_answer, db_files)                    \
    {                                                                        \
        CHECKER_TEST_FUNCTION(drop_test(request, checker_request,                 \
                                   checker_exception, checker_answer,        \
                                   db_files, client))                        \
    }

class Server {
   public:
    static Server* get() {
        if (obj_ == nullptr) {
            obj_ = new Server();
        }
        return obj_;
    }

    void run();
    void stop();

   private:
    Server() {}
    static Server* obj_;
    const std::string directory_ = "..\\Server\\";
    const std::string name_ = "OurSQL_Server";
};

void clearDB();
std::string check_request(const std::string& request, const long exception,
                          const std::string& answer, Client& client);
std::string drop_test(const std::string& request,
                      const std::string& checker_request,
                      const long checker_exception, const std::string& checker_answer,
                      const std::string& db_files, Client& client,
                      size_t start_time = 70, size_t step_time = 10, size_t max_time = 60000);
std::string get_select_answer(
    const std::vector<std::string>& column,
    const std::vector<std::vector<std::string>>& data);
std::string to_string(double a);

#endif
