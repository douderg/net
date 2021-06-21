#include <boost/asio/io_context.hpp>
#include <boost/beast/core/bind_handler.hpp>
#include <boost/beast/websocket/rfc6455.hpp>
#include <chrono>
#include <net/wss.hpp>
#include <net/async.hpp>

#include <memory>
#include <exception>
#include <stdexcept>
#include <iostream>
#include <thread>

namespace wss {

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

std::future<void> connection::write(const std::string& req) {
    if (!stream_) {
        throw std::runtime_error("not connected");
    }
    auto writer = std::make_shared<async::writer>();
    stream_->async_write(boost::asio::buffer(req), boost::beast::bind_front_handler(&async::writer::on_write, writer));
    return writer->result.get_future();
}

std::future<std::string> connection::read() {
    if (!stream_) {
        throw std::runtime_error("not connected");
    }
    auto reader = std::make_shared<async::reader>();
    stream_->async_read(reader->buffer, boost::beast::bind_front_handler(&async::reader::on_read, reader));
    return reader->result.get_future();
}

connection::operator bool() const {
    return bool(stream_);
}

void connection::close() {
    if (stream_) {
        try {
            auto dc = std::make_shared<disconnector>(stream_);
            dc->run().get();
        } catch (const std::runtime_error& err) {
            throw err;
        }
    }
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
        if (!SSL_set_tlsext_host_name(stream_.next_layer().native_handle(), host_.c_str())) {
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
        stream_.next_layer().async_handshake(
            boost::asio::ssl::stream_base::client,
            boost::beast::bind_front_handler(&connector::on_ssl_handshake, shared_from_this())
        );
    }
}

void connection::connector::on_ssl_handshake(boost::beast::error_code ec) {
    if (ec) {
        result.set_exception(std::make_exception_ptr(std::runtime_error(ec.message())));
    } else {
        boost::beast::get_lowest_layer(stream_).expires_never();
        stream_.set_option(boost::beast::websocket::stream_base::timeout::suggested(boost::beast::role_type::client));
        stream_.set_option(boost::beast::websocket::stream_base::decorator([this](boost::beast::websocket::request_type& req) -> void {
            req.set(boost::beast::http::field::host, host_);
            req.set(boost::beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);
        }));
        stream_.async_handshake(
            host_ + ":" + port_, "/", boost::beast::bind_front_handler(&connector::on_handshake, shared_from_this())
        );
    }
}

void connection::connector::on_handshake(boost::beast::error_code ec) {
    if (ec) {
        result.set_exception(std::make_exception_ptr(std::runtime_error(ec.message())));
    } else {
        connection conn;
        conn.stream_ = std::make_unique<boost::beast::websocket::stream<boost::beast::ssl_stream<boost::beast::tcp_stream>>>(std::move(stream_));
        result.set_value(std::move(conn));
    }
}

connection::disconnector::disconnector(std::unique_ptr<stream_t>& stream): stream_(stream.release()) {
    
}

std::future<void> connection::disconnector::run() {
    stream_->async_close(boost::beast::websocket::close_code::normal, boost::beast::bind_front_handler(&disconnector::on_shutdown, shared_from_this()));
    return result.get_future();
}

void connection::disconnector::on_shutdown(boost::beast::error_code ec) {
    if (ec) {
        result.set_exception(std::make_exception_ptr(std::runtime_error(ec.message())));
    } else {
        result.set_value();
    }
}

}