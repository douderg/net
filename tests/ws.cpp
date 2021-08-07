#include "net/http.hpp"
#include <cstdlib>
#include <future>
#include <net/client.hpp>
#include <net/server.hpp>
#include <stdexcept>
#include <iostream>


int server_side() {
    try {
        net::server server;
        auto ws = server.ws("127.0.0.1", 42425).get();
        ws.write("hello").get();
        std::string s = ws.read().get();
        ws.write("hello " + s).get();
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

        net::client client;
        auto ws = client.ws("127.0.0.1", "42425", "/");
        std::string s = ws.read().get();
        if (s != "hello") {
            std::cerr << "unexpected server message: " << s << "\n";
            return EXIT_FAILURE;
        }
        ws.write("client").get();
        std::string reply = ws.read().get();
        if (reply != "hello client") {
            std::cerr << "unexpected server message: " << s << "\n";
            return EXIT_FAILURE;
        }
        return server_result.get();
    } catch (const std::runtime_error& err) {
        std::cerr << err.what() << "\n";
        return EXIT_FAILURE;
    }
}