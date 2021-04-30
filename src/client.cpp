#include "https-client/client.hpp"
#include <iostream>

namespace https {

connection::connection(
    boost::asio::io_context& io_ctx, 
    boost::asio::ssl::context& ssl_ctx, 
    const std::string& sni_hostname, 
    boost::asio::ip::tcp::resolver::results_type resolved):
        stream_(io_ctx, ssl_ctx) {
    if (!SSL_set_tlsext_host_name(stream_.native_handle(), sni_hostname.c_str())) {
        throw std::runtime_error("failed setting SNI host name");
    }
    boost::beast::get_lowest_layer(stream_).connect(resolved);
    stream_.handshake(boost::asio::ssl::stream_base::client);
}

connection::~connection() {
    try {
        close();
    } catch (const std::runtime_error err) {
    }
}

response_t connection::send(const request_t& request) {
    boost::beast::http::write(stream_, request);
    boost::beast::flat_buffer buffer;
    response_t result;
    boost::beast::http::read(stream_, buffer, result);
    return result;
}

void connection::close() {
    boost::beast::error_code err;
    stream_.shutdown(err);
    if (err != boost::asio::error::eof) {
        throw std::runtime_error("failed to shutdown connection");
    }
    err = {};
}

client::client():
        ssl_ctx_(boost::asio::ssl::context::tlsv12_client),
        resolver_(io_ctx_) {
    ssl_ctx_.set_verify_mode(boost::asio::ssl::verify_peer);
    ssl_ctx_.set_default_verify_paths();
}

client::~client() {

}

connection client::connect(const std::string& host, const std::string& port) {
    return connection(io_ctx_, ssl_ctx_, host, resolver_.resolve(host, port));
}

}