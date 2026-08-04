#pragma once

#include <boost/asio.hpp>

#include <deque>
#include <filesystem>

namespace fstcp {
    namespace fs = std::filesystem;
    namespace asio = boost::asio;
    using asio::ip::tcp;

    class Session : public std::enable_shared_from_this<Session> {
    private:
        tcp::socket socket_;
        asio::streambuf stream_buffer_;

        std::deque<std::string> write_queue_;
        bool writing_ = false;

        fs::path current_dir_;

        bool closing_ = false;

        bool in_file_view_ = false;

        void do_read();
        void on_read(const boost::system::error_code &ec, std::size_t bytes_transferred);
        void queue_write(std::string message);
        void do_write();
        void close();

        void handle_line(std::string line);
        void handle_command(const std::string &line);

    public:
        explicit Session(tcp::socket socket);

        void start();
    };

}
