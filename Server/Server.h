#ifndef OURSQL_SERVER_H
#define OURSQL_SERVER_H

#define ASIO_STANDALONE

#include <iostream>
#include <mutex>
#include <queue>
#include <thread>

#include "asio.hpp"

#include "Our.h"

class Executer;

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
    Session(asio::ip::tcp::socket socket, std::shared_ptr<Executer> executer,
            LogOut& out)
        : tcp_socket_(std::move(socket)), executer_(executer), out_(out) {
        out_.write("Start of session: " + getSocketName());
    }
    ~Session() {
        out_.write("End of session: " + getSocketName());
    }
    void start() { read(); }
    void write(const std::string& response);

   private:
    std::string getSocketName();
    void read();

    LogOut& out_;
    std::shared_ptr<Executer> executer_;
    std::array<char, 1024> data_;
    asio::ip::tcp::socket tcp_socket_;
};

class Server {
   public:
    Server(asio::io_service& context, const short port,
           std::shared_ptr<Executer> executer, LogOut& out);

   private:
    void accept() {
        tcp_acceptor_.async_accept(tcp_socket_, [this](std::error_code ec) {
            if (!ec)
                std::make_shared<Session>(std::move(tcp_socket_), executer_,
                                          out_)
                    ->start();
            accept();
        });
    }
    asio::ip::tcp::acceptor tcp_acceptor_;
    asio::ip::tcp::socket tcp_socket_;
    std::shared_ptr<Executer> executer_;
    LogOut& out_;
};

class Executer : public std::enable_shared_from_this<Executer> {
   public:
    Executer(LogOut& out) : out_(out) {}
    ~Executer() {
        if (executer_.joinable()) {
            executer_.detach();
        }
    }
    void add(std::pair<std::string, std::shared_ptr<Session>> pair) {
        queue_lock_.lock();
        queue_.push(std::move(pair));
        queue_lock_.unlock();
    }
    void run();

   private:
    LogOut& out_;
    std::thread executer_;
    std::queue<std::pair<std::string, std::shared_ptr<Session>>> queue_;
    std::mutex queue_lock_;
};

void run_server(const short port, std::ostream& log = std::cout);

#endif  // OURSQL_SERVER_H
