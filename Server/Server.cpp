#include "Server.h"

void Session::write(const std::string& response) {
    auto self(shared_from_this());

    out_lock_->lock();
    out_ << getSocketName() << ". Write:\n" << response << std::endl;
    out_lock_->unlock();

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
                std::string ans = std::string(self->data_.data(), length) +
                                  std::string(a.data(), a.size());

                self->out_lock_->lock();
                self->out_ << self->getSocketName() << ". Get: " << ans
                           << std::endl;
                self->out_lock_->unlock();

                self->executer_->add({ans, self});
            }
        });
}

Server::Server(asio::io_service& context, const short port,
               std::shared_ptr<Executer> executer, std::ostream& out)
    : tcp_acceptor_(context,
                    asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)),
      tcp_socket_(context),
      executer_(executer),
      out_(out), out_lock_(std::make_shared<std::mutex>()) {
    out_lock_->lock();
    out_ << "Start of server." << std::endl;
    out_lock_->unlock();
    executer_->run();
    accept();
}

void Executer::run() {
    auto self(shared_from_this());
    executer_ = std::thread([self]() {
        while (1) {
            self->queue_lock_.lock();
            bool is_empty = self->queue_.empty();
            self->queue_lock_.unlock();

            if (!is_empty) {
                self->queue_lock_.lock();
                std::pair<std::string, std::shared_ptr<Session>> i =
                    self->queue_.front();
                self->queue_.pop();
                self->queue_lock_.unlock();

                std::stringstream in(i.first);
                std::stringstream out;
                std::string ans = std::to_string(ourSQL::perform(in, out));

                i.second->write(ans + " " + out.str());
            }
        }
    });
}

void run_server(const short port) {
    try {
        asio::io_context context;

        Server srv(context, port, std::make_shared<Executer>(), std::cout);

        context.run();
    } catch (std::exception& e) {
        std::cout << e.what() << std::endl;
    }
}
