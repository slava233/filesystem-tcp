#include <iostream>

#include <boost/asio.hpp>

namespace fstcp {
    namespace asio = boost::asio;
    using asio::ip::tcp;
}

using namespace fstcp;

namespace {
    void do_accept(tcp::acceptor &acceptor) {
        //TODO
    }

    unsigned short parse_port(const char *arg) {
        int value = std::stoi(arg);
        if (value <= 0 || value > 65535) {
            throw std::out_of_range("Port is out of range");
        }
        return static_cast<unsigned short>(value);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Использование: " << argv[0] << " <port>\n";
        return 1;
    }

    unsigned short port;
    try {
        port = parse_port(argv[1]);
    } catch (const std::exception&) {
        std::cerr << "Некорректный номер порта: " << argv[1] << "\n";
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