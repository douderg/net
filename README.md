### Description

Utility library using Boost's [Beast library](https://github.com/boostorg/beast) under the hood to implement a https/wss client.
Most of the code is based on the existing examples written by Vinnie Falco with a couple of tweaks to fit my needs.

The interfaces are not particularly stable as I am still learning myself so I have to tweak things around.

### Basic usage

```
#include <net/client.hpp>
#include <iostream>

int main() {
    // the client object holds the boost::asio io and ssl contexts
    // it needs to be kept alive while the connection is used
    
    net::client c;
    
    std::string url = ...; // your target host url here
    std::string port = "443"; // port number

    auto conn = c.http(url, port);
    
    https::request_t req(boost::beast::http::verb::get, "/index.html", 11);
    req.set(boost::beast::http::field::host, url);
    req.set(boost::beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);

    auto response = conn.send(req);
    if (response.result() != boost::beast::http::status::ok) {
        std::cerr << "error " << static_cast<int>(response.result()) << "\n";
        return 1;
    }
    
    std::cout << boost::beast::make_printable(response.body().data()) << "\n";
    
    return 0;
}
```
