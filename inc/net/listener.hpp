#pragma once

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

namespace net {

class listener {
public:
    listener(boost::asio::io_context& io_ctx, const std::string& host, uint16_t port);
protected:
    boost::asio::ip::tcp::acceptor acceptor_;
};

class ssl_listener : public listener {
public:
    ssl_listener(boost::asio::io_context& io_ctx, boost::asio::ssl::context& ssl_ctx, const std::string& host, uint16_t port);
protected:
    boost::asio::ssl::context& ssl_ctx_;
};

}