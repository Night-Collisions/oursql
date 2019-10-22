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

#define EXCEPTION2NUMB(expect) static_cast<long>(expect)

#define CHECK_REQUEST(request_message, exception, answer, client)           \
    {                                                                       \
        std::string out;                                                    \
        const std::string error_message =                                   \
            "Error in request:\n    " +                                     \
            ::testing::PrintToString(request_message) + "\n";               \
        long exception_request = client.request(request_message, out);      \
        if (exception_request != static_cast<long>(exception)) {            \
            FAIL() << error_message                                         \
                   << "Wrong exception code:\n  expected:\n    "            \
                   << static_cast<long>(exception) << "\n  real:\n    "     \
                   << exception_request << "\n  Exception message:\n    "   \
                   << ::testing::PrintToString(out);                        \
        }                                                                   \
        if (out != answer) {                                                \
            FAIL() << error_message                                         \
                   << "Wrong exception message:\n  expected:\n    "         \
                   << ::testing::PrintToString(answer) << "\n  real:\n    " \
                   << ::testing::PrintToString(out);                        \
        }                                                                   \
    }

#define CHECK_REQUEST_ST_CLIENT(request_message, exception, answer) \
    { CHECK_REQUEST(request_message, exception, answer, client); }

class Server {
   public:
    static Server* get() {
        if (obj_ == nullptr) {
            obj_ = new Server();
        }
        return obj_; }

    void run();
    void stop();

   private:
    Server() {}
    static Server* obj_;
    const std::string directory_ = "..\\Server\\";
    const std::string name_ = "OurSQL_Server";
};

void clearDB();

#endif
