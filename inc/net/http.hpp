#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <memory>

namespace http {

typedef boost::beast::http::response<boost::beast::http::dynamic_body> response_t;
typedef boost::beast::http::request<boost::beast::http::string_body> request_t;

class connection {
public:
    connection() = default;
    connection(boost::asio::io_context& io_ctx, const std::string& sni_hostname, boost::asio::ip::tcp::resolver::results_type resolve);
    connection(boost::asio::ip::tcp::socket socket);
    ~connection();
    connection(const connection&) = delete;
    connection(connection&&);
    connection& operator=(const connection&) = delete;
    connection& operator=(connection&&);

    response_t send(const request_t& request);

    void close();

    explicit operator bool() const;

private:
    std::unique_ptr<boost::beast::tcp_stream> stream_;
};

}
