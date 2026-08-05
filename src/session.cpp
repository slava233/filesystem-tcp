/* TCP session instance implementation.
 * FILE NAME: session.cpp
 * PROGRAMMER: Sviatoslav Kononov.
 * LAST UPDATE: 05.08.2026
 */

#include "session.hpp"
#include "helpers.hpp"

namespace fstcp {
    /* Session class explicit constructor
     * ARGUMENTS:
     *     - TCP socket:
     *         tcp::socket socket;
     */
    Session::Session(tcp::socket socket) : socket_(std::move(socket)) {}

    /* Read user input from TCP connection.
     * ARGUMENTS: None.
     * RETURNS: None.
     */
    void Session::do_read() {
        auto self = shared_from_this();
        asio::async_read_until(
            socket_, stream_buffer_, '\n',
            [this, self](const boost::system::error_code &ec, std::size_t bytes_transferred) {
                on_read(ec, bytes_transferred);
            });
    }

    /* User input processing.
     * ARGUMENTS:
     *     - error code:
     *         const boost::system::error_code &ec;
     *     - user input size:
     *         std::size_t bytes_transferred;
     * RETURNS: None.
     */
    void Session::on_read(const boost::system::error_code &ec, std::size_t) {
        // Exit if there is an error
        if (ec) {
            close();
            return;
        }

        // Process user input
        std::istream is(&stream_buffer_);
        std::string line;
        std::getline(is, line);

        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        handle_line(std::move(line));

        // Check if user closed the connection
        if (closing_) {
            close();
            return;
        }

        // Keep reading user input
        do_read();
    }

    /* Add a message into server's output.
     * ARGUMENTS:
     *     - message to send:
     *         std::string message;
     * RETURNS: None.
     */
    void Session::queue_write(std::string message) {
        if (message.empty()) {
            return;
        }

        write_queue_.push_back(std::move(message));

        do_write();
    }

    /* Add a message into server's output.
     * ARGUMENTS:
     *     - message to send:
     *         std::string message;
     * RETURNS: None.
     */
    void Session::do_write() {
        // Check if server is currently busy or there's nothing to send
        if (writing_ || write_queue_.empty()) {
            return;
        }

        writing_ = true;

        // Server sends an output
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

    /* Close TCP socket connection.
     * ARGUMENTS: None.
     * RETURNS: None.
     */
    void Session::close() {
        boost::system::error_code ec;
        socket_.shutdown(tcp::socket::shutdown_both, ec);
        socket_.close(ec);
    }

    /* Process the input received.
     * ARGUMENTS:
     *     - user input:
     *         std::string line;
     * RETURNS: None.
     */
    void Session::handle_line(std::string line) {
        std::string trimmed = trim(line);

        // Handle commands when user is in file view mode
        if (in_file_view_) {
            auto cmd = to_lower(trimmed);
            if (cmd == "q") {
                file_view_exit();
            } else if (cmd.empty()) {
                print_file_line();
            }
            return;
        }

        // Process the input as a general command
        if (trimmed.empty()) {
            return;
        }

        handle_command(trimmed);
    }

    /* Process the command received.
     * ARGUMENTS:
     *     - command to execute:
     *         std::string line;
     * RETURNS: None.
     */
    void Session::handle_command(const std::string &line) {
        // Extract command and arguments
        auto [cmd_raw, rest] = split_command(line);
        std::string cmd = to_lower(cmd_raw);

        // Execute corresponding supported command
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

    /* Print working directory.
     * ARGUMENTS: None.
     * RETURNS: None.
     */
    void Session::pwd() {
        queue_write(current_dir_.string() + "\n");
    }

    /* List directories and files in working directory.
     * ARGUMENTS: None.
     * RETURNS: None.
     */
    void Session::ls() {
        // Get lists of directories and files in working directory
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

        // Sort and print out lists of directories and files
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

    /* Change directory.
     * ARGUMENTS:
     *     - directory to move into:
     *         const std::string &path;
     * RETURNS: None.
     */
    void Session::cd(const std::string &path) {
        // Check for directory name
        if (path.empty()) {
            queue_write("cd: не указан путь\n");
            return;
        }

        // Check if path to this directory exists and change working directory
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

    /* View file content.
     * ARGUMENTS:
     *     - name of a file to view:
     *         const std::string &filename;
     * RETURNS: None.
     */
    void Session::more(const std::string &filename) {
        // Check for filename
        if (filename.empty()) {
            queue_write("more: не указан путь\n");
            return;
        }

        // Check if file exists
        fs::path file = current_dir_ / filename;

        std::error_code ec;
        if (!fs::is_regular_file(file, ec) || ec) {
            queue_write("more: файл не найден\n");
            return;
        }

        // Enter file viewing mode
        file_view_enter(file);
    }

    /* Enter file viewing mode.
     * ARGUMENTS:
     *     - name of a file to view:
     *         const fs::path &path;
     * RETURNS: None.
     */
    void Session::file_view_enter(const fs::path &path) {
        // Close any previously opened files
        file_stream_.close();
        file_stream_.clear();

        // Open a file
        file_stream_.open(path);
        if (!file_stream_.is_open()) {
            queue_write("more: не удалось открыть файл\n");
            return;
        }

        in_file_view_ = true;

        // Print first 20 lines in a file
        for (int i = 0; i < 20; i++) {
            print_file_line();
            if (file_stream_.peek() == EOF) {
                print_file_line();
                break;
            }
        }
    }

    /* Exit file viewing mode.
     * ARGUMENTS: None.
     * RETURNS: None.
     */
    void Session::file_view_exit() {
        in_file_view_ = false;
        file_stream_.close();
        file_stream_.clear();
    }

    /* Print next line in opened file.
     * ARGUMENTS: None.
     * RETURNS: None.
     */
    void Session::print_file_line() {
        std::string line;
        if (std::getline(file_stream_, line)) {
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            queue_write(line + "\n");
        } else {
            queue_write("--- КОНЕЦ ФАЙЛА ---\n");
        }
    }

    /* Start session.
     * ARGUMENTS: None.
     * RETURNS: None.
     */
    void Session::start() {
        current_dir_ = fs::current_path();
        do_read();
    }
}
