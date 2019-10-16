#ifndef OURSQL_CLIENT_H
#define OURSQL_CLIENT_H

#include <string>
#include "asio.hpp"

class Client {
   public:
    Client(const std::string& host, const short port) : tcp_socket_(context_), resolver_(context_) {
        asio::connect(tcp_socket_, resolver_.resolve({host.data(), std::to_string(port)}));
    }

    unsigned int request(const std::string& request, std::string& ans);

   protected:
    asio::io_context context_;
    asio::ip::tcp::socket tcp_socket_;
    asio::ip::tcp::resolver resolver_;
};

void run_client(const std::string& host, const short port);

#endif  // OURSQL_CLIENT_H
