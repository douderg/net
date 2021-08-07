# pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/websocket.hpp>
#include <memory>
#include <future>

namespace net {
class client;
class server;
}

namespace ws {

class connection {
public:
    connection() = default;
    connection(
        boost::asio::io_context& io_ctx, 
        const std::string& sni_hostname, 
        const std::string& port, 
        const std::string& uri, 
        boost::asio::ip::tcp::resolver::results_type resolve
    );
    
    ~connection();
    connection(const connection&) = delete;
    connection(connection&&);
    connection& operator=(const connection&) = delete;
    connection& operator=(connection&&);

    std::future<std::string> read();
    std::future<void> write(const std::string& data);

    explicit operator bool() const;

    void close();
private:

    friend class net::client;
    friend class net::server;

    typedef boost::beast::websocket::stream<boost::beast::tcp_stream> stream_t;
    class connector;
    class disconnector;
    class acceptor;

    std::unique_ptr<stream_t> stream_;
};


class connection::connector : public std::enable_shared_from_this<connection::connector> {
    
    stream_t stream_;
    std::string host_, port_;
public:
    connector(boost::asio::io_context& io_ctx, const std::string& host, const std::string& port);

    void on_resolve(boost::beast::error_code ec, boost::asio::ip::tcp::resolver::results_type results);
    void on_connect(boost::beast::error_code ec, boost::asio::ip::tcp::resolver::results_type::endpoint_type endpoint);
    void on_handshake(boost::beast::error_code ec);

    std::promise<connection> result;
};


class connection::disconnector : public std::enable_shared_from_this<connection::disconnector> {
    
    std::unique_ptr<stream_t> stream_;
    std::promise<void> result;

public:
    disconnector(std::unique_ptr<stream_t>& stream);

    std::future<void> run();
    void on_shutdown(boost::beast::error_code ec);
};


class connection::acceptor : public std::enable_shared_from_this<connection::acceptor> {
    boost::asio::ip::tcp::acceptor acceptor_;
    std::unique_ptr<stream_t> stream_;
public:
    acceptor(boost::asio::io_context& io_ctx, const std::string& host, uint16_t port);

    void on_accept(boost::beast::error_code ec, boost::asio::ip::tcp::socket socket);
    void on_dispatch();
    void on_websocket_accept(boost::beast::error_code ec);
    
    std::promise<connection> result;
};

}