#define ASIO_STANDALONE

#include <iostream>
#include <string>
#include "asio.hpp"

#include "Client.h"

std::string get_request(std::istream& in = std::cin) {
    int c = 0;
    std::string req;
    while (((c = in.get()) != EOF) && (req.back() != ';')) {
        req.push_back(c);
    }
    return req;
}

void run_client(const std::string& host, const short port) {
    try {
        asio::io_context context;
        asio::ip::tcp::socket tcp_socket(context);
        asio::ip::tcp::resolver resolver(context);
        asio::connect(tcp_socket,
                      resolver.resolve({host.data(), std::to_string(port)}));

        while (1) {
            const size_t max_message_length = 1024;
            std::string s = get_request();
            tcp_socket.write_some(asio::buffer(s.data(), s.length()));

            std::array<char, max_message_length> reply;
            size_t bytes_readable = 0;
            std::string ans;
            do {
                size_t reply_length =
                    tcp_socket.read_some(asio::buffer(reply, reply.size()));
                ans += std::string(reply.data(), reply_length);
                asio::socket_base::bytes_readable command(true);
                tcp_socket.io_control(command);
                bytes_readable = command.get();
            } while (bytes_readable != 0);
            std::cout << "reply: ";
            std::cout << ans << std::endl;
        }

    } catch (std::exception& e) {
        std::cerr << "!" << e.what() << std::endl;
    }
}
