#include <boost/asio/executor_work_guard.hpp>
#include <net/client.hpp>
#include <net/cert_utils.hpp>

#include <boost/beast.hpp>
#include <boost/asio.hpp>

#include <thread>
#include <iostream>

namespace net {

client::client():
        work_guard_(boost::asio::make_work_guard(io_ctx_)), // prevents io_ctx_ from stopping when there is no work to do
        ssl_ctx_(boost::asio::ssl::context::tlsv12_client),
        resolver_(boost::asio::make_strand(io_ctx_)) {
#ifdef _WIN32
    import_openssl_certificates(ssl_ctx_.native_handle());
#endif
    ssl_ctx_.set_verify_mode(boost::asio::ssl::verify_peer);
    ssl_ctx_.set_default_verify_paths();
    bg_ = std::thread([this]() -> void {
        io_ctx_.run();
    });
}

client::~client() {
    io_ctx_.stop();
    bg_.join();
}

http::connection client::http(const std::string& host, const std::string& port) {
    return http::connection(io_ctx_, host, resolver_.resolve(host, port));
}

ws::connection client::ws(const std::string& host, const std::string& port, const std::string& uri) {
    return ws::connection(io_ctx_, host, port, uri, resolver_.resolve(host, port));
}

https::connection client::https(const std::string& host, const std::string& port) {
    return https::connection(io_ctx_, ssl_ctx_, host, resolver_.resolve(host, port));
}

std::future<wss::connection> client::wss(const std::string& host, const std::string& port, const std::string& uri) {
    auto connector = std::make_shared<wss::connection::connector>(io_ctx_, ssl_ctx_, host, port);
    resolver_.async_resolve(
        host, port, boost::beast::bind_front_handler(&wss::connection::connector::on_resolve, connector)
    );
    return connector->result.get_future();
}

}