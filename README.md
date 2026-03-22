# net++

net++ is a class-based C++ encapsulation over POSIX sockets providing a simple, readable interface, asynchronous support, and RAII management for persistent objects like connections.

This project is still under development, but these features are planned/currently in the works:
- Event-driven I/O handling (multiplexing) using libevent for blocking calls such as TCP sends and receives.
- Support for UDP sockets
- Abstractions for connection pooling


## Quick Start

### Connecting to a host
Connecting to a host involves two simple steps. Client-sided applications use the `conn_resolver::Resolver` class to get a list of valid `AddressInfo` results. The `conn_resolver::Resolver::try_connect()` method attempts to connect to a result and returns a shared pointer to the first connection.

```cpp
conn_resolver::Resolver res{ "www.google.com", "8080" };
tcp::Connection::connection_ptr conn = res.try_connect();

conn->send_sync(...);
conn->recv_sync(...);

```

### Accepting incoming connections
Setting up a listening connection is similar to the process with the POSIX API. First, create a `tcp::Acceptor` instance with a port to listen on and the address domain the underlying socket will use to listen on. As usual, you can call `tcp::Acceptor::bind()` to resolve the underlying socket to the host machine, `tcp::Acceptor::listen()` to listen for incoming connections, and `tcp::Acceptor::accept()` to create dedicated sockets for incoming connections. 

```cpp
tcp::Acceptor acceptor{ "8080", networking::domain::ipv4 }

acceptor.bind();
acceptor.listen();
Connection::connection_ptr conn{ acceptor.accept() };

...
conn->send_sync(...);
conn->recv_sync(...);
...
```

You can view the full documentation [here.](http://jpagcal.github.io/net-plus-plus)

## Error Handling
Errors are surfaced as `std::system_error` whenever low-level POSIX calls set the value of `errno`.

Not all errors are exceptional (See [Kohlhoff, 2010](http://blog.think-async.com/2010/04/system-error-support-in-c0x-part-1.html)) -- a good example would be calling `connect()` to attempt connections to a host iteratively through a list of query results. Hence, system calls that are executed within an iterative sequence, which also have some reasonable expectation of failure, like `bind()` or `connect()` are exempt from throwing if they fail. If all system calls fail for all results, then a `std::system_error` is emitted.

It is the responsibility of the library user to handle exceptions as follows:

```cpp
...
try {
	// throwing code
} catch (const std::system_error& e) {
	// exception handling code here
}
...
```

For convenience, the `log_error(std::system_error &e)` function, which logs descriptions of errors to stderr, is made available in `error.hpp`
