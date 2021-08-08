#pragma once

#include <boost/beast.hpp>
#include <boost/beast/core/bind_handler.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/core/stream_traits.hpp>
#include <boost/beast/http/dynamic_body.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/read.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/http/write.hpp>
#include <exception>
#include <future>
#include <memory>
#include <stdexcept>
#include <utility>
#include <functional>

namespace async {

struct reader {
    boost::beast::flat_buffer buffer;
    std::promise<std::string> result;

    void on_read(boost::beast::error_code ec, size_t bytes);
};

struct writer {
    std::promise<void> result;
    void on_write(boost::beast::error_code ec, size_t bytes);
};

template <class StreamT, class RequestT, class ResponseT>
class request_handler : public std::enable_shared_from_this<request_handler<StreamT, RequestT, ResponseT>> {
public:
    request_handler(StreamT& stream, std::function<ResponseT(const RequestT&)> func): stream_(stream), handler_(func) {
        
    }

    std::future<void> handle_next() {
        request_ = {};
        boost::beast::http::async_read(
            stream_, 
            buffer_, 
            request_, 
            boost::beast::bind_front_handler(&request_handler<StreamT, RequestT, ResponseT>::on_read, this->shared_from_this())
        );
        return result_.get_future();
    }

private:
    void on_read(boost::beast::error_code ec, size_t bytes) {
        if (ec) {
            result_.set_exception(std::make_exception_ptr(std::runtime_error(ec.message())));
        } else {
            response_ = handler_(request_);
            boost::beast::http::async_write(
                stream_, 
                response_, 
                boost::beast::bind_front_handler(&request_handler<StreamT, RequestT, ResponseT>::on_write, this->shared_from_this(), request_.need_eof())
            );
        }
    }

    void on_write(bool close, boost::beast::error_code ec, size_t bytes) {
        if (ec) {
            result_.set_exception(std::make_exception_ptr(std::runtime_error(ec.message())));
        } else {
            if (close) {
                boost::beast::error_code ec;
                boost::beast::get_lowest_layer(stream_).socket().shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);
            }
            result_.set_value();
        }
    }

    StreamT& stream_;
    std::function<ResponseT(const RequestT&)> handler_;
    boost::beast::flat_buffer buffer_;
    std::promise<void> result_;

    RequestT request_;
    ResponseT response_;
};

}