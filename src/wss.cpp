#include <https-client/wss.hpp>


namespace wss {

connection::connection(
        boost::asio::io_context& io_ctx, 
        boost::asio::ssl::context& ssl_ctx, 
        const std::string& sni_hostname, 
        const std::string& port,
        const std::string& uri, 
        boost::asio::ip::tcp::resolver::results_type resolved) {
    
    stream_ = std::make_unique<boost::beast::websocket::stream<boost::beast::ssl_stream<boost::beast::tcp_stream>>>(io_ctx, ssl_ctx);
    if (!SSL_set_tlsext_host_name(stream_->next_layer().native_handle(), sni_hostname.c_str())) {
        throw std::runtime_error("failed setting SNI host name");
    }
    boost::beast::get_lowest_layer(*stream_).connect(resolved);
    stream_->next_layer().handshake(boost::asio::ssl::stream_base::client);
    stream_->handshake(sni_hostname + ":" + port, uri);
    auto change_user_agent = [=](boost::beast::websocket::request_type& req)->void {
        req.set(boost::beast::http::field::host, sni_hostname);
        req.set(boost::beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    };
    stream_->set_option(boost::beast::websocket::stream_base::decorator(change_user_agent));
}

connection::~connection() {
    try {
        close();
    } catch (const std::runtime_error&) {
    }
}

connection::connection(connection&& other): stream_{other.stream_.release()} {
    
}

connection& connection::operator=(connection&& other) {
    stream_.reset(other.stream_.release());
    return *this;
}

void connection::write(const std::string& data) {
    stream_->write(boost::asio::buffer(data));
}

std::string connection::read() {
    boost::beast::flat_buffer buffer;
    stream_->read(buffer);
    return boost::beast::buffers_to_string(buffer.data());
}

struct async_reader {
    boost::beast::flat_buffer buffer;
    std::promise<std::string> result;

    void on_read(boost::beast::error_code ec, size_t bytes) {
        if (ec) {
            result.set_exception(std::make_exception_ptr(std::runtime_error(ec.message())));
            return;
        }
        result.set_value(boost::beast::buffers_to_string(buffer.data()));
    }
};

std::future<std::string> connection::async_read() {
    auto reader = std::make_shared<async_reader>();
    stream_->async_read(reader->buffer, boost::beast::bind_front_handler(&async_reader::on_read, reader));
    return reader->result.get_future();
}

connection::operator bool() const {
    return bool(stream_);
}

void connection::close() {
    if (stream_) {
        try {
            stream_->close(boost::beast::websocket::close_code::normal);
        } catch (const std::runtime_error& err) {
            stream_.reset(nullptr);
            throw err;
        }
        stream_.reset(nullptr);
    }
}

}