#pragma once

#include <boost/asio/executor_work_guard.hpp>
#include <boost/asio/io_context.hpp>
#include <net/http.hpp>
#include <net/https.hpp>
#include <net/ws.hpp>
#include <net/wss.hpp>


namespace net {

class client {
public:
    client();
    ~client();

    boost::asio::ssl::context& ssl_context();

    http::connection http(const std::string& host, const std::string& port);
    std::future<ws::connection> ws(const std::string& host, const std::string& port, const std::string& uri);

    https::connection https(const std::string& host, const std::string& port);
    std::future<wss::connection> wss(const std::string& host, const std::string& port, const std::string& uri);

private:
    boost::asio::io_context io_ctx_;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_guard_;
    boost::asio::ssl::context ssl_ctx_;
    boost::asio::ip::tcp::resolver resolver_;
    std::thread bg_;
};

}