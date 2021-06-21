#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <memory>

namespace https {

typedef boost::beast::http::response<boost::beast::http::dynamic_body> response_t;
typedef boost::beast::http::request<boost::beast::http::string_body> request_t;

class connection {
public:
    connection() = default;
    connection(boost::asio::io_context& io_ctx, boost::asio::ssl::context& ssl_ctx, const std::string& sni_hostname, boost::asio::ip::tcp::resolver::results_type resolve);
    ~connection();
    connection(const connection&) = delete;
    connection(connection&&);
    connection& operator=(const connection&) = delete;
    connection& operator=(connection&&);

    response_t send(const request_t& request);

    void close();

    explicit operator bool() const;

private:
    std::unique_ptr<boost::beast::ssl_stream<boost::beast::tcp_stream>> stream_;
};

}