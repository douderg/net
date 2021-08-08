#include <boost/asio/dispatch.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/beast/core/bind_handler.hpp>
#include <boost/beast/core/stream_traits.hpp>
#include <boost/beast/http/write.hpp>
#include <chrono>
#include <net/https.hpp>
#include <net/async.hpp>
#include <boost/asio/strand.hpp>

namespace https {

connection::connection(
        boost::asio::io_context& io_ctx, 
        boost::asio::ssl::context& ssl_ctx, 
        const std::string& sni_hostname, 
        boost::asio::ip::tcp::resolver::results_type resolved) {
    
    stream_ = std::make_unique<boost::beast::ssl_stream<boost::beast::tcp_stream>>(boost::asio::make_strand(io_ctx), ssl_ctx);
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

std::future<void> connection::handle_request(std::function<https::response_t(const https::request_t&)> func) {
    if (!stream_) {
        throw std::runtime_error("no connection");
    }
    return std::make_shared<async::request_handler<stream_t, https::request_t, https::response_t>>(*stream_, func)->handle_next();
}


connection::connector::connector(boost::asio::io_context& io_ctx, boost::asio::ssl::context& ssl_ctx, const std::string& host, const std::string& port): 
        stream_(boost::asio::make_strand(io_ctx), ssl_ctx),
        host_(host),
        port_(port) {

}

void connection::connector::on_resolve(boost::beast::error_code ec, boost::asio::ip::tcp::resolver::results_type results) {
    if (ec) {
        result.set_exception(std::make_exception_ptr(std::runtime_error(ec.message())));
    } else {
        if (!SSL_set_tlsext_host_name(stream_.native_handle(), host_.c_str())) {
            result.set_exception(std::make_exception_ptr(std::runtime_error("failed setting SNI host name")));
            return;
        }
        boost::beast::get_lowest_layer(stream_).async_connect(
            results, 
            boost::beast::bind_front_handler(&connector::on_connect, shared_from_this())
        );
    }
}

void connection::connector::on_connect(boost::beast::error_code ec, boost::asio::ip::tcp::resolver::results_type::endpoint_type endpoint) {
    if (ec) {
        result.set_exception(std::make_exception_ptr(std::runtime_error(ec.message())));
    } else {
        stream_.async_handshake(
            boost::asio::ssl::stream_base::client,
            boost::beast::bind_front_handler(&connector::on_handshake, shared_from_this())
        );
    }
}

void connection::connector::on_handshake(boost::beast::error_code ec) {
    if (ec) {
        result.set_exception(std::make_exception_ptr(std::runtime_error(ec.message())));
    } else {
        connection conn;
        conn.stream_ = std::make_unique<stream_t>(std::move(stream_));
        result.set_value(std::move(conn));
    }
}

connection::listener::listener(boost::asio::io_context& io_ctx, boost::asio::ssl::context& ssl_ctx, const std::string& host, uint16_t port): 
        acceptor_(boost::asio::make_strand(io_ctx)),
        ssl_ctx_(ssl_ctx) {
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
    auto result = std::make_shared<async_result>(ssl_ctx_);
    acceptor_.async_accept(boost::beast::bind_front_handler(&async_result::on_accept, result));
    return result->result.get_future();
}

connection::listener::async_result::async_result(boost::asio::ssl::context& ssl_ctx):
        ssl_ctx_(ssl_ctx) {

}

void connection::listener::async_result::on_accept(boost::beast::error_code ec, boost::asio::ip::tcp::socket socket) {
    if (ec) {
        result.set_exception(std::make_exception_ptr(std::runtime_error(ec.message())));
    } else {
        stream_ = std::make_unique<stream_t>(std::move(socket), ssl_ctx_);
        boost::asio::dispatch(
            stream_->get_executor(), 
            boost::beast::bind_front_handler(&async_result::on_dispatch, shared_from_this())
        );
    }
}

void connection::listener::async_result::on_dispatch() {
    boost::beast::get_lowest_layer(*stream_).expires_after(std::chrono::seconds(30));
    stream_->async_handshake(
        boost::asio::ssl::stream_base::server, 
        boost::beast::bind_front_handler(&async_result::on_handshake, shared_from_this())
    );
}

void connection::listener::async_result::on_handshake(boost::beast::error_code ec) {
    if (ec) {
        result.set_exception(std::make_exception_ptr(std::runtime_error(ec.message())));
    } else {
        connection conn;
        conn.stream_ = std::move(stream_);
        result.set_value(std::move(conn));
    }
}

}