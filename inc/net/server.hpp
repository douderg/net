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
    std::future<ws::connection> ws(const std::string& host, uint16_t port);
    std::future<wss::connection> wss(const std::string& host, uint16_t port);

private:
    boost::asio::io_context io_ctx_;
    boost::asio::ssl::context ssl_ctx_;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_guard_;
    std::thread worker_;

    // std::shared_ptr<https::connection::listener> https_;
    // std::shared_ptr<ws::connection::listener> ws_;
    // std::shared_ptr<wss::connection::listener> wss_;
};

};