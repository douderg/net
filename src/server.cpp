#include <boost/asio/strand.hpp>
#include <boost/beast/core/bind_handler.hpp>
#include <memory>
#include <net/server.hpp>

namespace net {

server::server():work_guard_(boost::asio::make_work_guard(io_ctx_)), ssl_ctx_(boost::asio::ssl::context::tlsv12_server) {
    worker_ = std::thread([this]() -> void {
        io_ctx_.run();
    });
}

server::~server() {
    io_ctx_.stop();
    worker_.join();
}

std::shared_ptr<http::connection::listener> server::http(const std::string& host, uint16_t port) {
    return std::make_shared<http::connection::listener>(io_ctx_, host, port);
}

std::shared_ptr<https::connection::listener> server::https(const std::string& host, uint16_t port) {
    return std::make_shared<https::connection::listener>(io_ctx_, ssl_ctx_, host, port);
}

std::future<ws::connection> server::ws(const std::string& host, uint16_t port) {
    return std::make_shared<ws::connection::acceptor>(io_ctx_, host, port)->result.get_future();
}

std::future<wss::connection> server::wss(const std::string& host, uint16_t port) {
    return std::make_shared<wss::connection::acceptor>(io_ctx_, ssl_ctx_, host, port)->result.get_future();
}

}