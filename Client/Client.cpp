#include "Client.h"

#include <iostream>

size_t number_digit(unsigned int n) {
    size_t k;
    n /= 10;
    for (k = 1; n > 0; n /= 10, k++) {
    }
    return k;
}

long Client::request(const std::string& request, std::string& ans) {
    auto send_exception = sendRequest(request);
    if (send_exception != 0) {
        return send_exception;
    }
    return getAnswer(ans);
}

long Client::sendRequest(const std::string& request) {
    try {
        tcp_socket_.write_some(asio::buffer(request.data(), request.length()));
    } catch (...) {
        serverException();
        return -1;
    }
    return 0;
}

long Client::getAnswer(std::string& ans) {
    int code_exception = -1;
    ans = std::string();
    try {
        std::array<char, 1024> reply;
        size_t bytes_readable = 0;
        do {
            size_t reply_length =
                tcp_socket_.read_some(asio::buffer(reply, reply.size()));
            ans += std::string(reply.data(), reply_length);
            asio::socket_base::bytes_readable command(true);
            tcp_socket_.io_control(command);
            bytes_readable = command.get();
        } while (bytes_readable != 0);

        std::istringstream(ans) >> code_exception;
        if (code_exception >= 0) {
            ans.erase(0,
                      number_digit(code_exception) + 1);
        } else {
            assert("Server response failed.");
        }
    } catch (...) {
        serverException();
        return -1;
    }
    return code_exception;
}

void Client::connect() {
    out_ << "Trying to connect to the server..." << std::endl;
    try {
        asio::connect(tcp_socket_, resolver_.resolve(query_));
    } catch (...) {
        out_ << "Attempt failed." << std::endl;
        if (is_repeat_server_connection_) {
            connect();
        }
        return;
    }
    out_ << "Server connection established." << std::endl;
}

void Client::serverException() {
    out_ << "Request failed: server is not available!" << std::endl;
    if (is_reconnect_on_server_exception_) {
        connect();
    }
}
