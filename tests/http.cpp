

#include "net/http.hpp"
#include <boost/beast/http/field.hpp>
#include <boost/beast/http/status.hpp>
#include <boost/beast/http/verb.hpp>
#include <boost/beast/version.hpp>
#include <chrono>
#include <cstdlib>
#include <future>
#include <net/client.hpp>
#include <net/server.hpp>
#include <stdexcept>
#include <iostream>
#include <thread>

int server_side() {
    auto req_handler = [](const http::request_t& req) -> http::response_t {
        http::response_t result;
        result.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
        result.keep_alive(req.keep_alive());
        result.version(req.version());
        if (req.method() != boost::beast::http::verb::get) {
            result.result(boost::beast::http::status::method_not_allowed);
        } else {
            if (req.target() == "/test1") {
                result.result(boost::beast::http::status::ok);
            } else {
                result.result(boost::beast::http::status::not_found);
            }
        }
        return result;
    };

    try {
        net::server server;
        auto listener = server.http("127.0.0.1", 42428);
        listener->accept_next().get()
            .handle_request(req_handler).get();
        listener->accept_next().get()
            .handle_request(req_handler).get();
        
    } catch (const std::runtime_error& err) {
        std::cerr << err.what() << "\n";
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

http::request_t make_request(const std::string& target) {
    http::request_t req{};
    req.version(11);
    req.keep_alive(true);
    req.target(target);
    req.method(boost::beast::http::verb::get);
    req.prepare_payload();
    return req;
}

int main(int argc, char** argv) {
    
    try {
        auto server_result = std::async(std::launch::async, []()-> int {
            return server_side();
        });
        
        net::client client;

        auto response = client.http("127.0.0.1", "42428").send(make_request("/test1"));
        if (response.result() != boost::beast::http::status::ok) {
            std::cerr << "unexpected response " << response.result() << "\n";
            return EXIT_FAILURE;
        }

        response = client.http("127.0.0.1", "42428").send(make_request("/test2"));
        if (response.result() != boost::beast::http::status::not_found) {
            std::cerr << "unexpected response\n";
            return EXIT_FAILURE;
        }
        
        return server_result.get();
    } catch (const std::runtime_error& err) {
        std::cerr << err.what() << "\n";
        return EXIT_FAILURE;
    }
}