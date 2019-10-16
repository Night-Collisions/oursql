#include "Client.h"

#include <iostream>

size_t number_digit(unsigned int n) {
    size_t k;
    for (k = 0; n > 0; n /= 10) {
    }
    return k;
}

long Client::request(const std::string& request, std::string& ans) {
    int code_exception = -1;
    try {
        ans = std::string();
        tcp_socket_.write_some(asio::buffer(request.data(), request.length()));

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
                      number_digit(code_exception) + 2);
        } else {
            assert("Server response failed.");
        }
    } catch (...) {
        out_ << "Request failed: server is not available!" << std::endl;
        connect();
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
        connect();
        return;
    }
    out_ << "Server connection established." << std::endl;
}
