#include <net/client.hpp>
#include <net/cert_utils.hpp>
#include <boost/beast/version.hpp>


namespace net {

client::client():
        ssl_ctx_(boost::asio::ssl::context::tlsv12_client),
        resolver_(io_ctx_) {
#ifdef _WIN32
    import_openssl_certificates(ssl_ctx_.native_handle());
#endif
    ssl_ctx_.set_verify_mode(boost::asio::ssl::verify_peer);
    ssl_ctx_.set_default_verify_paths();
}

client::~client() {

}

http::connection client::http(const std::string& host, const std::string& port) {
    return http::connection(io_ctx_, host, resolver_.resolve(host, port));
}

ws::connection client::ws(const std::string& host, const std::string& port, const std::string& uri) {
    return ws::connection(io_ctx_, host, port, uri, resolver_.resolve(host, port));
}

https::connection client::https(const std::string& host, const std::string& port) {
    return https::connection(io_ctx_, ssl_ctx_, host, resolver_.resolve(host, port));
}

wss::connection client::wss(const std::string& host, const std::string& port, const std::string& uri) {
    return wss::connection(io_ctx_, ssl_ctx_, host, port, uri, resolver_.resolve(host, port));
}

}