#pragma once

#include <net/http.hpp>
#include <net/https.hpp>
#include <net/ws.hpp>
#include <net/wss.hpp>


namespace net {

class client {
public:
    client();
    ~client();
    http::connection http(const std::string& host, const std::string& port);
    ws::connection ws(const std::string& host, const std::string& port, const std::string& uri);

    https::connection https(const std::string& host, const std::string& port);
    wss::connection wss(const std::string& host, const std::string& port, const std::string& uri);
private:
    boost::asio::io_context io_ctx_;
    boost::asio::ssl::context ssl_ctx_;
    boost::asio::ip::tcp::resolver resolver_;
};

}