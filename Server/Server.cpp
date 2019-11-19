#include "Server.h"

#include <ctime>
#include <iomanip>


namespace ourSQL {
namespace server {

void LogOut::write(const std::string& s) {
    auto time = std::time(nullptr);
    auto local_time = *std::localtime(&time);
    lock_.lock();
    str_ << std::put_time(&local_time, "%d-%m-%Y_%H-%M-%S::") << s << std::endl;
    lock_.unlock();
}

void Session::write(const std::string& response) {
    auto self(shared_from_this());

    out_.write(getSocketName() + ". Write: \"" + response + "\".");

    tcp_socket_.async_write_some(
        asio::buffer(response.data(), response.length()),
        [self](std::error_code ec, const std::size_t) {
            if (!ec) {
                self->read();
            }
        });
}

std::string Session::getSocketName() {
    return tcp_socket_.remote_endpoint().address().to_string() + ":" +
           std::to_string(tcp_socket_.remote_endpoint().port());
}

void Session::read() {
    auto self(shared_from_this());

    tcp_socket_.async_read_some(
        asio::buffer(data_, data_.size()),
        [self](std::error_code ec, std::size_t length) {
            if (!ec) {
                asio::socket_base::bytes_readable command(true);
                self->tcp_socket_.io_control(command);
                size_t bytes_readable = command.get();

                std::vector<char> a(bytes_readable);
                auto b = asio::buffer(a.data(), a.size());
                self->tcp_socket_.read_some(b);
                std::string request = std::string(self->data_.data(), length) +
                                      std::string(a.data(), a.size());

                self->out_.write(self->getSocketName() + ". Get: \"" + request +
                                 "\".");

                std::stringstream in(request);
                std::stringstream out;
                std::string ans = std::to_string(ourSQL::perform(in, out));
                self->out_.write("Was executed: \"" + request +
                                 "\". Answer: \"" + out.str() + "\".");

                self->write(ans + " " + out.str());
            }
        });
}

Server::Server(asio::io_service& context, const short port, LogOut& out)
    : tcp_acceptor_(context,
                    asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)),
      tcp_socket_(context),
      out_(out) {
    out_.write("Start of server!");
    accept();
}

void run_server(const short port, std::ostream& log) {
    LogOut out(log);
    try {
        asio::io_context context;

        Server srv(context, port, out);

        context.run();
    } catch (std::exception& e) {
        out.write("Server error: " + std::string(e.what()));
    }
}

}  // namespace server
}  // namespace ourSQL
