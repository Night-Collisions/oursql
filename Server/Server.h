#ifndef OURSQL_SERVER_H
#define OURSQL_SERVER_H

#define ASIO_STANDALONE

#include <iostream>
#include <mutex>
#include <queue>
#include <thread>

#include "asio.hpp"

#include "Our.h"

namespace ourSQL {
namespace server {

class LogOut {
   public:
    LogOut(std::ostream& str) : str_(str) {}
    void write(const std::string& s);

   protected:
    std::ostream& str_;
    std::mutex lock_;
};

class Session : public std::enable_shared_from_this<Session> {
   public:
    Session(asio::ip::tcp::socket socket, LogOut& out)
        : tcp_socket_(std::move(socket)), out_(out) {
        out_.write("Start of session: " + getSocketName());
    }
    ~Session() { out_.write("End of session: " + getSocketName()); }
    void start() { read(); }
    void write(const std::string& response);

   private:
    std::string getSocketName();
    void read();

    LogOut& out_;
    std::array<char, 1024> data_;
    asio::ip::tcp::socket tcp_socket_;
};

class Server {
   public:
    Server(asio::io_service& context, const short port, LogOut& out);

   private:
    void accept() {
        tcp_acceptor_.async_accept(tcp_socket_, [this](std::error_code ec) {
            if (!ec)
                std::make_shared<Session>(std::move(tcp_socket_), out_)
                    ->start();
            accept();
        });
    }
    asio::ip::tcp::acceptor tcp_acceptor_;
    asio::ip::tcp::socket tcp_socket_;
    LogOut& out_;
};

void run_server(const short port, std::ostream& log = std::cout);

}  // namespace server
}  // namespace ourSQL

#endif  // OURSQL_SERVER_H
