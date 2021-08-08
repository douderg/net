### Description

Utility library using Boost's [Beast library](https://github.com/boostorg/beast) under the hood to implement a simple https/wss client/server.
Most of the code is based on the existing examples written by Vinnie Falco with a couple of tweaks to suit my needs. More specifically, most async operations are performed in a background thread with a std::future object returned to the main thread for performing error handling there.

### Basic usage

The interfaces are not particularly stable as I am still learning myself so I have to keep tweaking things around.

Check the source files in the tests directory for code samples that can be considered up to date.
