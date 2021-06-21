#include <net/async.hpp>


namespace async {

void reader::on_read(boost::beast::error_code ec, size_t bytes) {
    if (ec) {
        result.set_exception(std::make_exception_ptr(std::runtime_error(ec.message())));
        return;
    }
    result.set_value(boost::beast::buffers_to_string(buffer.data()));
}

void writer::on_write(boost::beast::error_code ec, size_t bytes) {
    if (ec) {
        result.set_exception(std::make_exception_ptr(std::runtime_error(ec.message())));
        return;
    }
    result.set_value();
}

}