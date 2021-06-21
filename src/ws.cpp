#include <net/ws.hpp>
#include <net/async.hpp>

#include <boost/beast/version.hpp>


namespace ws {

connection::connection(
        boost::asio::io_context& io_ctx, 
        const std::string& host, 
        const std::string& port,
        const std::string& uri, 
        boost::asio::ip::tcp::resolver::results_type resolved) {
    
    stream_ = std::make_unique<boost::beast::websocket::stream<boost::beast::tcp_stream>>(io_ctx);
    stream_->next_layer().connect(resolved);
    
    auto change_user_agent = [=](boost::beast::websocket::request_type& req)->void {
        req.set(boost::beast::http::field::host, host);
        req.set(boost::beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    };
    stream_->set_option(boost::beast::websocket::stream_base::decorator(change_user_agent));

    stream_->handshake(host + ":" + port, uri);
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



std::future<std::string> connection::async_read() {
    auto reader = std::make_shared<async::reader>();
    stream_->async_read(reader->buffer, boost::beast::bind_front_handler(&async::reader::on_read, reader));
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