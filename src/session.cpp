#include "session.hpp"
#include "helpers.hpp"

namespace fstcp {
    Session::Session(tcp::socket socket) : socket_(std::move(socket)) {}

    void Session::do_read() {
        auto self = shared_from_this();
        asio::async_read_until(
            socket_, stream_buffer_, '\n',
            [this, self](const boost::system::error_code &ec, std::size_t bytes_transferred) {
                on_read(ec, bytes_transferred);
            });
    }

    void Session::on_read(const boost::system::error_code &ec, std::size_t) {
        if (ec) {
            close();
            return;
        }

        std::istream is(&stream_buffer_);
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
        //TODO file view mode

        std::string trimmed = trim(line);

        if (trimmed.empty()) {
            return;
        }

        handle_command(trimmed);
    }

    void Session::handle_command(const std::string &line) {
        auto [cmd_raw, rest] = split_command(line);
        std::string cmd = to_lower(cmd_raw);

        if (cmd == "pwd") {
            pwd();
        } else if (cmd == "ls") {
            ls();
        } else if (cmd == "cd") {
            cd(rest);
        } else if (cmd == "more") {
            more(rest);
        } else if (cmd == "exit") {
            closing_ = true;
        } else {
            queue_write("Hеизвестная команда\n");
        }
    }

    void Session::pwd() {
        queue_write(current_dir_.string() + "\n");
    }

    void Session::ls() {
        std::vector<std::string> dirs, files;

        try {
            for (const auto &entry : fs::directory_iterator(current_dir_)) {
                std::string name = entry.path().filename().string();
                if (entry.is_directory()) {
                    dirs.push_back(name + "/");
                } else {
                    files.push_back(name);
                }
            }
        } catch (const fs::filesystem_error&) {
            queue_write("ls: не удалось прочитать директорию\n");
            return;
        }

        std::sort(dirs.begin(), dirs.end());
        std::sort(files.begin(), files.end());

        std::string out;
        for (const auto &d : dirs) {
            out += d + "\n";
        }
        for (const auto &f : files) {
            out += f + "\n";
        }

        queue_write(out);
    }

    void Session::cd(const std::string &path) {
        if (path.empty()) {
            queue_write("cd: не указан путь\n");
            return;
        }

        fs::path target = current_dir_ / path;

        std::error_code ec;
        fs::path new_path = fs::weakly_canonical(target, ec);
        if (ec) {
            queue_write("cd: нет такой директории\n");
            return;
        }

        if (!fs::is_directory(new_path, ec) || ec) {
            queue_write("cd: нет такой директории\n");
            return;
        }

        current_dir_ = new_path;
    }

    void Session::more(const std::string &filename) {
        if (filename.empty()) {
            queue_write("more: не указан путь\n");
            return;
        }

        fs::path target = current_dir_ / filename;

        std::error_code ec;
        if (!fs::is_regular_file(target, ec) || ec) {
            queue_write("more: файл не найден\n");
            return;
        }

        //TODO file view mode
    }

    void Session::start() {
        current_dir_ = fs::current_path();
        do_read();
    }
}
