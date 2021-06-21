#include <net/http.hpp>


namespace http {

connection::connection(
        boost::asio::io_context& io_ctx, 
        const std::string& sni_hostname, 
        boost::asio::ip::tcp::resolver::results_type resolved) {
    
    stream_ = std::make_unique<boost::beast::tcp_stream>(io_ctx);
    stream_->connect(resolved);
}

connection::connection(boost::asio::ip::tcp::socket socket) {
    stream_ = std::make_unique<boost::beast::tcp_stream>(std::move(socket));
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
            stream_->socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both);
        } catch (const std::runtime_error& err) {
            stream_.reset(nullptr);
            throw err;
        }
        stream_.reset(nullptr);
    }
}

}