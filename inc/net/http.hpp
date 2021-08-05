#pragma once

#include <net/async.hpp>

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/http/message.hpp>
#include <memory>
#include <future>
#include <functional>

namespace net {
class client;
class server;
}

namespace http {

typedef boost::beast::http::response<boost::beast::http::dynamic_body> response_t;
typedef boost::beast::http::request<boost::beast::http::string_body> request_t;

class connection {
    typedef boost::beast::tcp_stream stream_t;
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

    std::future<void> handle_request(std::function<http::response_t(const http::request_t&)> func);

    class listener;

private:

    friend class net::client;
    friend class net::server;
    
    class connector;

    std::unique_ptr<stream_t> stream_;
    std::function<response_t(const request_t&)> on_request_;
    std::promise<void> next_response_;
    request_t next_request_;
    boost::beast::flat_buffer buffer_;
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

class connection::listener : public std::enable_shared_from_this<connection::listener> {
public:
    listener(boost::asio::io_context& io_ctx, const std::string& host, uint16_t port);
    std::future<connection> accept_next();
private:
    void on_accept(boost::beast::error_code ec, boost::asio::ip::tcp::socket socket);

    boost::asio::ip::tcp::acceptor acceptor_;
    std::promise<connection> connection_;
};

}
