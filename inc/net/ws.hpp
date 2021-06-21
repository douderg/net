# pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/websocket.hpp>
#include <memory>
#include <future>

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

    void write(const std::string& data);
    std::string read();

    std::future<std::string> async_read();

    explicit operator bool() const;

    void close();
private:
    std::unique_ptr<boost::beast::websocket::stream<boost::beast::tcp_stream>> stream_;
};

}