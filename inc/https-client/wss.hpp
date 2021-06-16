#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>

namespace wss {

class connection {
public:
    connection() = default;
    connection(
        boost::asio::io_context& io_ctx, 
        boost::asio::ssl::context& ssl_ctx, 
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

    void write(const std::string& data);
    std::string read();

    explicit operator bool() const;

    void close();
private:
    std::unique_ptr<boost::beast::websocket::stream<boost::beast::ssl_stream<boost::beast::tcp_stream>>> stream_;
};

}