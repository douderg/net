#include <https-client/client.hpp>
#include <https-client/cert_utils.hpp>
#include <iostream>

namespace https {

connection::connection(
        boost::asio::io_context& io_ctx, 
        boost::asio::ssl::context& ssl_ctx, 
        const std::string& sni_hostname, 
        boost::asio::ip::tcp::resolver::results_type resolved) {
    
    stream_ = std::make_unique<boost::beast::ssl_stream<boost::beast::tcp_stream>>(io_ctx, ssl_ctx);
    if (!SSL_set_tlsext_host_name(stream_->native_handle(), sni_hostname.c_str())) {
        throw std::runtime_error("failed setting SNI host name");
    }
    boost::beast::get_lowest_layer(*stream_).connect(resolved);
    stream_->handshake(boost::asio::ssl::stream_base::client);
}

connection::~connection() {
    try {
        close();
    } catch (const std::runtime_error& err) {
    }
}

connection::connection(connection&& other): stream_{other.stream_.release()} {
    
}

connection& connection::operator=(connection&& other) {
    stream_.reset(other.stream_.release());
    return *this;
}

response_t connection::send(const request_t& request) {
    if (!stream_) {
        throw std::runtime_error("connection is closed");
    }
    boost::beast::http::write(*stream_, request);
    boost::beast::flat_buffer buffer;
    response_t result;
    boost::beast::http::read(*stream_, buffer, result);
    return result;
}

void connection::close() {
    if (stream_) {
        stream_->shutdown();
        stream_.reset(nullptr);
    }
}

client::client():
        ssl_ctx_(boost::asio::ssl::context::tlsv12_client),
        resolver_(io_ctx_) {
#ifdef _WIN32
    import_openssl_certificates(ssl_ctx_.native_handle());
#endif
    ssl_ctx_.set_verify_mode(boost::asio::ssl::verify_peer);
    ssl_ctx_.set_default_verify_paths();
}

client::~client() {

}

connection client::connect(const std::string& host, const std::string& port) {
    return connection(io_ctx_, ssl_ctx_, host, resolver_.resolve(host, port));
}

wss::connection client::ws(const std::string& host, const std::string& port, const std::string& uri) {
    return wss::connection(io_ctx_, ssl_ctx_, host, port, uri, resolver_.resolve(host, port));
}

}