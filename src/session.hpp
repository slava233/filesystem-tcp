/* TCP session instance declaration.
 * FILE NAME: session.hpp
 * PROGRAMMER: Sviatoslav Kononov.
 * LAST UPDATE: 05.08.2026
 */

#pragma once

#include <boost/asio.hpp>

#include <deque>
#include <filesystem>
#include <fstream>

namespace fstcp {
    namespace fs = std::filesystem;
    namespace asio = boost::asio;
    using asio::ip::tcp;

    // TCP session instance class
    class Session : public std::enable_shared_from_this<Session> {
    private:
        tcp::socket socket_;                     // TCP socket
        asio::streambuf stream_buffer_;          // User input

        std::deque<std::string> write_queue_;    // Server output
        bool writing_ = false;                   // Server writing status

        fs::path current_dir_;                   // User current directory

        bool closing_ = false;                   // User closing connection

        bool in_file_view_ = false;              // User file view mode status
        std::ifstream file_stream_;              // Read file data

        // TCP connection functions

        /* Read user input from TCP connection.
         * ARGUMENTS: None.
         * RETURNS: None.
         */
        void do_read();

        /* User input processing.
         * ARGUMENTS:
         *     - error code:
         *         const boost::system::error_code &ec;
         *     - user input size:
         *         std::size_t bytes_transferred;
         * RETURNS: None.
         */
        void on_read(const boost::system::error_code &ec, std::size_t bytes_transferred);

        /* Add a message into server's output.
         * ARGUMENTS:
         *     - message to send:
         *         std::string message;
         * RETURNS: None.
         */
        void queue_write(std::string message);

        /* Add a message into server's output.
         * ARGUMENTS:
         *     - message to send:
         *         std::string message;
         * RETURNS: None.
         */
        void do_write();

        /* Close TCP socket connection.
         * ARGUMENTS: None.
         * RETURNS: None.
         */
        void close();

        // Process user input

        /* Process the input received.
         * ARGUMENTS:
         *     - user input:
         *         std::string line;
         * RETURNS: None.
         */
        void handle_line(std::string line);

        /* Process the command received.
         * ARGUMENTS:
         *     - command to execute:
         *         std::string line;
         * RETURNS: None.
         */
        void handle_command(const std::string &line);

        // Supported commands

        /* Print working directory.
         * ARGUMENTS: None.
         * RETURNS: None.
         */
        void pwd();

        /* List directories and files in working directory.
         * ARGUMENTS: None.
         * RETURNS: None.
         */
        void ls();

        /* Change directory.
         * ARGUMENTS:
         *     - directory to move into:
         *         const std::string &path;
         * RETURNS: None.
         */
        void cd(const std::string &path);

        /* View file content.
         * ARGUMENTS:
         *     - name of a file to view:
         *         const std::string &filename;
         * RETURNS: None.
         */
        void more(const std::string &filename);

        // File view functions

        /* Enter file viewing mode.
         * ARGUMENTS:
         *     - name of a file to view:
         *         const fs::path &path;
         * RETURNS: None.
         */
        void file_view_enter(const fs::path &path);

        /* Exit file viewing mode.
         * ARGUMENTS: None.
         * RETURNS: None.
         */
        void file_view_exit();

        /* Print next line in opened file.
         * ARGUMENTS: None.
         * RETURNS: None.
         */
        void print_file_line();

    public:
        /* Session class explicit constructor
         * ARGUMENTS:
         *     - TCP socket:
         *         tcp::socket socket;
         */
        explicit Session(tcp::socket socket);

        /* Start session.
         * ARGUMENTS: None.
         * RETURNS: None.
         */
        void start();
    };

}
