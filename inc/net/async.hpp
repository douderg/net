#pragma once

#include <boost/beast.hpp>
#include <future>
#include <memory>
#include <utility>

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

}