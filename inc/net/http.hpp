#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <memory>
#include <future>

namespace net {
class client;
}

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

    friend class net::client;
    typedef boost::beast::tcp_stream stream_t;
    class connector;

    std::unique_ptr<stream_t> stream_;
};


class connection::connector : public std::enable_shared_from_this<connection::connector> {
    stream_t stream_;
    std::string host_, port_;
public:
    connector(boost::asio::io_context& io_ctx, const std::string& host, const std::string& port);

    void on_resolve(boost::beast::error_code ec, boost::asio::ip::tcp::resolver::results_type results);
    void on_connect(boost::beast::error_code ec, boost::asio::ip::tcp::resolver::results_type::endpoint_type endpoint);

    std::promise<connection> result;
};

}
