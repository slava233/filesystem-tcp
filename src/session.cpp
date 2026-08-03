#include "session.hpp"

namespace fstcp {
    Session::Session(tcp::socket socket) : socket_(std::move(socket)) {}

    void Session::do_read() {
        auto self = shared_from_this();
        asio::async_read_until(
            socket_, input_buffer_, '\n',
            [this, self](const boost::system::error_code &ec, std::size_t bytes_transferred) {
                on_read(ec, bytes_transferred);
            });
    }

    void Session::on_read(const boost::system::error_code &ec, std::size_t) {
        if (ec) {
            close();
            return;
        }

        std::istream is(&input_buffer_);
        std::string line;
        std::getline(is, line);

        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        handle_line(std::move(line));

        if (closing_) {
            close();
            return;
        }

        do_read();
    }

    void Session::queue_write(std::string message) {
        if (message.empty()) {
            return;
        }

        write_queue_.push_back(std::move(message));
        do_write();
    }

    void Session::do_write() {
        if (writing_ || write_queue_.empty()) {
            return;
        }

        writing_ = true;

        auto self = shared_from_this();
        asio::async_write(
            socket_, asio::buffer(write_queue_.front()),
            [this, self](const boost::system::error_code &ec, std::size_t) {
                writing_ = false;
                if (ec) {
                    close();
                    return;
                }
                write_queue_.pop_front();
                do_write();
            });
    }

    void Session::close() {
        boost::system::error_code ec;
        socket_.shutdown(tcp::socket::shutdown_both, ec);
        socket_.close(ec);
    }

    void Session::handle_line(std::string line) {
        //TODO
    }

    void Session::start() {
        current_dir_ = fs::current_path();
        do_read();
    }
}
