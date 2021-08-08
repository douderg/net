#pragma once

#include <boost/asio/ssl/context.hpp>
#include <net/http.hpp>
#include <net/https.hpp>
#include <net/ws.hpp>
#include <net/wss.hpp>

#include <boost/asio.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <future>

namespace net {

class server {
    
public:
    server();
    ~server();

    boost::asio::ssl::context& ssl_context();

    std::shared_ptr<http::connection::listener> http(const std::string& host, uint16_t port);
    std::shared_ptr<https::connection::listener> https(const std::string& host, uint16_t port);
    std::shared_ptr<ws::connection::listener> ws(const std::string& host, uint16_t port);
    std::future<wss::connection> wss(const std::string& host, uint16_t port);

    class listener;
    class ssl_listener;

private:
    boost::asio::io_context io_ctx_;
    boost::asio::ssl::context ssl_ctx_;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_guard_;
    std::thread worker_;
};

class server::listener {
public:
    listener(boost::asio::io_context& io_ctx, const std::string& host, uint16_t port);
protected:
    boost::asio::ip::tcp::acceptor acceptor_;
};

class server::ssl_listener : public listener {
public:
    ssl_listener(boost::asio::io_context& io_ctx, boost::asio::ssl::context& ssl_ctx, const std::string& host, uint16_t port);
private:
    boost::asio::ssl::context& ssl_ctx_;
};

};