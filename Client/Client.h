#ifndef OURSQL_CLIENT_H
#define OURSQL_CLIENT_H

#define ASIO_STANDALONE

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
          out_(out) {}

    long request(const std::string& request, std::string& ans);

    long sendRequest(const std::string& request);
    long getAnswer(std::string& ans);

    void connect();

    void setRpConnection(bool f) { is_repeat_server_connection_ = f; }
    void setExceptionReconnect(bool f) { is_reconnect_on_server_exception_ = f; }

   protected:
    void serverException();

    asio::io_context context_;
    asio::ip::tcp::socket tcp_socket_;
    asio::ip::tcp::resolver resolver_;
    asio::ip::tcp::resolver::query query_;
    std::ostream& out_;
    bool is_repeat_server_connection_ = true;
    bool is_reconnect_on_server_exception_ = false;
};

void run_client(const std::string& host, const short port);

#endif  // OURSQL_CLIENT_H
