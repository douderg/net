#include <net/listener.hpp>
#include <boost/beast.hpp>

namespace net {
listener::listener(boost::asio::io_context& io_ctx, const std::string& host, uint16_t port):
        acceptor_(boost::asio::make_strand(io_ctx)) {
    boost::beast::error_code ec;
    boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::make_address(host), port);
    acceptor_.open(endpoint.protocol(), ec);
    if (ec) {
        throw std::runtime_error(ec.message());
    }
    acceptor_.set_option(boost::beast::net::socket_base::reuse_address(true), ec);
    if (ec) {
        throw std::runtime_error(ec.message());
    }
    acceptor_.bind(endpoint, ec);
    if (ec) {
        throw std::runtime_error(ec.message());
    }
    acceptor_.listen(boost::beast::net::socket_base::max_listen_connections, ec);
    if (ec) {
        throw std::runtime_error(ec.message());
    }
}

ssl_listener::ssl_listener(boost::asio::io_context& io_ctx, boost::asio::ssl::context& ssl_ctx, const std::string& host, uint16_t port):
        listener(io_ctx, host, port),
        ssl_ctx_(ssl_ctx) {

}

}