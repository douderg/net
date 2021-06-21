#include <memory>
#include <net/http.hpp>
#include <boost/asio/strand.hpp>

namespace http {

connection::connection(
        boost::asio::io_context& io_ctx, 
        const std::string& sni_hostname, 
        boost::asio::ip::tcp::resolver::results_type resolved) {
    
    stream_ = std::make_unique<boost::beast::tcp_stream>(boost::asio::make_strand(io_ctx));
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


connection::connector::connector(boost::asio::io_context& io_ctx, const std::string& host, const std::string& port): 
        stream_(boost::asio::make_strand(io_ctx)),
        host_(host),
        port_(port) {

}

void connection::connector::on_resolve(boost::beast::error_code ec, boost::asio::ip::tcp::resolver::results_type results) {
    if (ec) {
        result.set_exception(std::make_exception_ptr(std::runtime_error(ec.message())));
    } else {
        stream_.async_connect(
            results, 
            boost::beast::bind_front_handler(&connector::on_connect, shared_from_this())
        );
    }
}

void connection::connector::on_connect(boost::beast::error_code ec, boost::asio::ip::tcp::resolver::results_type::endpoint_type endpoint) {
    if (ec) {
        result.set_exception(std::make_exception_ptr(std::runtime_error(ec.message())));
    } else {
        connection conn;
        conn.stream_ = std::make_unique<stream_t>(std::move(stream_));
        result.set_value(std::move(conn));
    }
}

}