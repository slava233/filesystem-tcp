#include "session.hpp"

#include <iostream>

using namespace fstcp;

namespace {
    void do_accept(tcp::acceptor &acceptor) {
        acceptor.async_accept([&acceptor](boost::system::error_code ec, tcp::socket socket) {
            if (!ec) {
                std::make_shared<Session>(std::move(socket))->start();
            }
            do_accept(acceptor);
        });
    }

    unsigned short parse_port(const char *arg) {
        int port = std::stoi(arg);
        if (port <= 0 || port > 65535) {
            throw std::out_of_range("Port is out of range");
        }
        return static_cast<unsigned short>(port);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Использование: " << argv[0] << " <порт>" << std::endl;
        return 1;
    }

    unsigned short port;
    try {
        port = parse_port(argv[1]);
    } catch (const std::exception&) {
        std::cerr << "Некорректный номер порта: " << argv[1] << std::endl;
        return 1;
    }

    try {
        asio::io_context io_context;
        tcp::acceptor acceptor(io_context, tcp::endpoint(asio::ip::make_address("127.0.0.1"), port));

        do_accept(acceptor);

        std::cout << "Сервер запущен на 127.0.0.1:" << port << std::endl;

        io_context.run();
    } catch (const std::exception &e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}