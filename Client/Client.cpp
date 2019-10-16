#define ASIO_STANDALONE

#include "Client.h"

#include <iostream>

size_t number_digit(unsigned int n) {
    size_t k;
    for (k = 0; n > 0; n %= 10) {}
    return k;
}


unsigned int Client::request(const std::string& request, std::string& ans) {
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

    unsigned int code_exception = 0;
    std::istringstream(ans) >> code_exception;
    ans.erase(0, number_digit(code_exception) + 1); // TODO: не работает в случае превышения, сервак не отвечает
    return code_exception;
}
