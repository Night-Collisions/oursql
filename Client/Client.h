#ifndef OURSQL_CLIENT_H
#define OURSQL_CLIENT_H

#include <iostream>
#include <string>

#include "asio.hpp"

class Client {
   public:
    Client(const std::string& host, const short port,
           std::ostream& out = std::cout)
        : tcp_socket_(context_),
          resolver_(context_),
          query_({host.data(), std::to_string(port)}),
          out_(out) {
        connect();
    }

    long request(const std::string& request, std::string& ans);

   protected:
    void connect();

    asio::io_context context_;
    asio::ip::tcp::socket tcp_socket_;
    asio::ip::tcp::resolver resolver_;
    asio::ip::tcp::resolver::query query_;
    std::ostream& out_;
};

void run_client(const std::string& host, const short port);

#endif  // OURSQL_CLIENT_H
