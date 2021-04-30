#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>

namespace https {

typedef boost::beast::http::response<boost::beast::http::dynamic_body> response_t;
typedef boost::beast::http::request<boost::beast::http::string_body> request_t;

class connection {
public:
    connection(boost::asio::io_context& io_ctx, boost::asio::ssl::context& ssl_ctx, const std::string& sni_hostname, boost::asio::ip::tcp::resolver::results_type resolve);
    ~connection();
    connection(const connection&) = delete;
    connection(connection&&) = default;
    connection& operator=(const connection&) = delete;
    connection& operator=(connection&&) = default;

    response_t send(const request_t& request);

    void close();

private:
    boost::beast::ssl_stream<boost::beast::tcp_stream> stream_;
};

class client {
public:
    client();
    ~client();
    connection connect(const std::string& host, const std::string& port);
private:
    boost::asio::io_context io_ctx_;
    boost::asio::ssl::context ssl_ctx_;
    boost::asio::ip::tcp::resolver resolver_;
};

}