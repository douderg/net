#include "net/async.hpp"
#include <boost/asio/ip/address.hpp>
#include <boost/asio/socket_base.hpp>
#include <boost/beast/core/bind_handler.hpp>
#include <boost/beast/core/error.hpp>
#include <boost/beast/http/dynamic_body.hpp>
#include <boost/beast/http/read.hpp>
#include <boost/beast/http/string_body.hpp>
#include <exception>
#include <memory>
#include <net/http.hpp>
#include <boost/asio/strand.hpp>
#include <stdexcept>

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

std::future<void> connection::handle_request(std::function<http::response_t(const http::request_t&)> func) {
    if (!stream_) {
        throw std::runtime_error("no connection");
    }
    return std::make_shared<async::request_handler<stream_t, http::request_t, http::response_t>>(*stream_, func)->handle_next();
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

connection::listener::listener(boost::asio::io_context& io_ctx, const std::string& host, uint16_t port): 
        acceptor_(boost::asio::make_strand(io_ctx)) {
    boost::beast::error_code ec;
    boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::make_address(host), port);
    acceptor_.open(endpoint.protocol(), ec);
    if (ec) {
        throw std::runtime_error(ec.message());
    }
    acceptor_.set_option(boost::beast::net::socket_base::reuse_address(true), ec);
    if (ec) {
        throw std::runtime_error(ec.message());
    }
    acceptor_.bind(endpoint, ec);
    if (ec) {
        throw std::runtime_error(ec.message());
    }
    acceptor_.listen(boost::beast::net::socket_base::max_listen_connections, ec);
    if (ec) {
        throw std::runtime_error(ec.message());
    }
}

std::future<connection> connection::listener::accept_next() {
    connection_ = std::promise<connection>();
    acceptor_.async_accept(boost::beast::bind_front_handler(&listener::on_accept, shared_from_this()));
    return connection_.get_future();
}

void connection::listener::on_accept(boost::beast::error_code ec, boost::asio::ip::tcp::socket socket) {
    if (ec) {
        connection_.set_exception(std::make_exception_ptr(std::runtime_error(ec.message())));
    } else {
        connection result;
        result.stream_ = std::make_unique<stream_t>(std::move(socket));
        connection_.set_value(std::move(result));
    }
}

}