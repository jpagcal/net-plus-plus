# net++

net++ is a class-based C++ encapsulation over POSIX sockets providing a simple, readable interface, asynchronous support, and RAII management for persistent objects like connections.

**__Current Progress__**
- [x] Encapsulations for sockaddr and addrinfo
- [x] Resolver class to abstract getaddr usage, as well as mechanisms to create TCP connections from query results
- [x] TCP connection class, with synchronous sends and receives
- [x] TCP acceptor class
- [ ] Event-driven I/O handling (multiplexing) using libevent for blocking calls such as TCP sends and receives.
	- [x] IOContext class
	- [ ] Define bufferevents for the Connection class
	- [ ] Integrate libevent with Acceptor
- [ ] Support for UDP sockets
- [ ] Abstractions for connection pooling


## Quick Start

### Connecting to a host
Connecting to a host involves two simple steps. Client-sided applications use the `conn_resolver::Resolver` class to get a list of valid `AddressInfo` results. The `conn_resolver::Resolver::try_connect()` method attempts to connect to a result and returns a shared pointer to the first connection.

```cpp
conn_resolver::Resolver res{ "www.google.com", "8080" };
tcp::Connection::connection_ptr conn = res.try_connect_tcp();

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

## A synchronous single-threaded TCP echo client and server implementation

**TCP Echo Server**
```cpp
int main() {
	std::string port{ "8080" };
	
	try {
		tcp::Acceptor acceptor{"8080", networking::domain::ipv4};
		
		acceptor.bind();
		acceptor.listen();
		std::cout << "Server listening on port " << port << '\n';
		
		tcp::Connection::connection_ptr conn{ acceptor.accept() };
		
		while (true) {
			std::string buf;
			conn->recv_sync(buf);
			conn->send_sync(buf);
		}
	} catch (std::system_error &e) {
		netpp::log_error(e);
	}
}
```

**TCP Echo Client**
```cpp
int main(){
	try {
		conn_resolver::Resolver res{ "127.0.0.1", "8080" };
		tcp::Connection::connection_ptr conn{ res.try_connect_tcp() };
		
	while (true) {
		std::string received{};
		std::string message{};
		std::getline(std::cin, message);
		conn->send_sync(message);
		conn->recv_sync(received);
		std::cout << "Received \"" << received << "\" from connection"<< std::endl;
	}
	} catch (std::system_error &e) {
		netpp_error::log_error(e);
	}
}
```

## Asynchronous Events
net++ uses [libevent](http://libevent.org) to execute callbacks on registered file descriptors - specifically with socket reads, writes, and accepting connections. The `IOContext` class defined in `io_context.hpp` manages the lifecycle and dispatch of the event loop. 

Events are automatically registered with the IOContext upon setting their handling objects as non-blocking. For example, if an instance of `tcp::Connection`, `conn` calls `conn.set_nonblocking()`, a buffer event monitoring the socket's internal buffers is registered with `IOContext`. An `IOContext` object is calling by calling the default constructor:

```cpp
async::IOContext async_context{};

...
// ...non-block object creation...
...

async_context.run();
```

The underlying event base that serves as the foundation for `IOContext` is not threadsafe by default, therefore `IOContext` is not threadsafe. Having multiple threads polling for IO implies setting up an `IOContext` per thread.
