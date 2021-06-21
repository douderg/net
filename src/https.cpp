#include <net/https.hpp>


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
    } catch (const std::runtime_error&) {
    }
}

connection::connection(connection&& other): stream_{other.stream_.release()} {
    
}

connection& connection::operator=(connection&& other) {
    stream_.reset(other.stream_.release());
    return *this;
}

connection::operator bool() const {
    return bool(stream_);
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
        try {
            stream_->shutdown();
        } catch (const std::runtime_error& err) {
            stream_.reset(nullptr);
            throw err;
        }
        stream_.reset(nullptr);
    }
}

}