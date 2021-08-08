#include <net/client.hpp>
#include <net/server.hpp>
#include <cert.hpp>

#include <chrono>
#include <cstdlib>
#include <future>
#include <stdexcept>
#include <iostream>
#include <thread>


int server_side() {
    try {
        net::server server;
        server.ssl_context().use_certificate_chain(
            boost::asio::buffer(cert.data(), cert.size())
        );
        server.ssl_context().use_private_key(
            boost::asio::buffer(key.data(), key.size()),
            boost::asio::ssl::context::file_format::pem
        );
        server.ssl_context().use_tmp_dh(boost::asio::buffer(dhparams.data(), dhparams.size()));
        auto ws = server.wss("127.0.0.1", 42426);
        auto session = ws->accept_next().get();
        session.write("hello");
        std::string s = session.read().get();
        session.write("hello " + s).get();
    } catch (const std::runtime_error& err) {
        std::cerr << err.what() << "\n";
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int main() {

    try {
        std::future<int> server_result = std::async(
            std::launch::async, 
            []() -> int { return server_side(); }
        );
        {
            net::client client;
            client.ssl_context().add_certificate_authority(
                boost::asio::buffer(cert.data(), cert.size())
            );
            auto ws = client.wss("127.0.0.1", "42426", "/").get();
            std::string s = ws.read().get();
            if (s != "hello") {
                std::cerr << "unexpected server message: " << s << "\n";
                return EXIT_FAILURE;
            }
            ws.write("client").get();
            std::string reply = ws.read().get();
            if (reply != "hello client") {
                std::cerr << "unexpected server message: " << reply << "\n";
                return EXIT_FAILURE;
            }
        }
        return server_result.get();
    } catch (const std::runtime_error& err) {
        std::cerr << err.what() << "\n";
        return EXIT_FAILURE;
    }
}