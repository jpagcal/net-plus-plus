#pragma once

#include <functional>
#include <memory>
#include <string_view>
#include <sys/socket.h>
#include "../include/io_context.hpp"
#include <event2/bufferevent.h>

namespace tcp {
/**
 * @brief Represents a TCP connection
 *
 * Encapsulates a connected socket, allowing send and receive capabilities, as well as
 * I/O multiplexing using libevent.
 */
class Connection : public std::enable_shared_from_this<Connection> {
	using length_header = int32_t;
	static constexpr size_t header_size{ sizeof(length_header )};
public:
	using connection_ptr = std::shared_ptr<Connection>;
	using event_ptr = std::unique_ptr<bufferevent, decltype(&bufferevent_free)>;
	/// @cond HIDDEN_FROM_DOCS
	Connection(const Connection&) = delete;
	Connection& operator=(const Connection&) = delete;
	/// @endcond

	/**
	 * @brief The move constructor for the Connection class
	 *
	 * Moves the source socket handle, io context, and event to the current Connection instance, and
	 * invalidates the source instance's socket handle.
	 *
	 * @param other An rvalue reference to the object to steal resources from
	 */
	Connection(Connection&& other) noexcept;
	/**
	 * @brief The move constructor for the Connection class
	 *
	 * Closes the current Connection's socket handle if applicable, moves the source
	 * socket handle, io context, and event to the current connection instance , and invalidates the source
     * instance's socket handle.
	 *
	 * @param other An rvalue reference to the object to steal resources from
	 */
	Connection &operator=(Connection&& other) noexcept;

	/**
	 * @brief Creates a shared ptr to a new Connection instance
		*
		* @param socket_fd The Connected socket file descriptor to pass into the private constructor
		* @param io_context An optional IOContext object
		* @returns A shared pointer to the new Connection instance
	 */
	static connection_ptr create(int32_t socket_fd, async::IOContext::io_context_ptr io_context);

	/**
	 * @brief Sets the underlying socket handle as nonblocking
		* and creates a bufferevent for the socket
	 */
	void set_nonblocking();

	/**
	 * @brief Checks if the underlying socket handle is nonblocking
		*
		* @returns A boolean indicating whether or not the socket is nonblocking
	 */
	bool is_nonblocking();

	/**
	 * @brief Sends a message through the endpoint asynchronously
		*
		* Only use when the socket is set to nonblocking. Asynchronously sends
		* all bytes in msg and calls the callback function when the operation is completed
		*
		* @param msg The buffer of bytes to send
		* @param callback The function to call once the send operation completes
	 */
	void send_async(std::string_view msg, std::function<void()> callback);

	/**
	 * @brief Receives a message from the endpoint asynchronously
		*
		* Only use when the socket is set to nonblocking. Asynchronously
		* receives all bytes into buf and calls the callback function when the operation is completed
		*
		* @param buf The buffer of bytes to receive data
		* @param callback The function to call once the receive operation completes
	 */
	void recv_async(std::vector<std::byte> buf, std::function<void()> callback);

	/**
	 * @brief Sends a message through the endpoint synchronously
		*
		* Only use when is_nonblocking() returns false. Synchronously
		* sends all bytes in msg.
		*
		* @param msg the buffer of bytes to send
	 */
	void send_sync(std::string_view msg);

	/**
	 * @brief Receives a message from the endpoint synchronously
		*
		* Only use when is_nonblocking() returns false. Synchronously receives
		* all bytes into buf.
		*
		* @param buf The buffer of bytes to receive data
	 */
	void recv_sync(std::string &buf);
private:
	/// @cond HIDDEN_FROM_DOCS
	Connection(int32_t socket_fd, async::IOContext::io_context_ptr io_context = nullptr) :
    	socket_fd_{ socket_fd },
     	io_context_{ io_context }, // TODO: should take in the io_context as an argument
      	event_(nullptr, bufferevent_free) {}
	/// @endcond

	int32_t socket_fd_; /**< A handle to the connected socket */
	std::shared_ptr<async::IOContext> io_context_; /**< The event loop */
	event_ptr event_; /**<The buffer event */
};

/**
 * @brief Accepts connections and assigns a dedicated TCPConnection object
 *
 * Encapsulates a listening socket
 */
class Acceptor {
public:
	/**
	 * @brief Constructor for the Acceptor object.
		*
		* Creates a TCP connection with the capability to listen and accept connections on the given port.
		*
		* @param port The port in string format
	 */
	Acceptor(std::string port, int32_t domain);

	/**
	 * @brief Destructor of the Acceptor object
		*
		* Closes the underlying socket handle.
	 */
	~Acceptor();

	/// @cond HIDDEN_FROM_DOCS
	Acceptor(const Acceptor&) = delete;
	Acceptor& operator=(const Acceptor&) = delete;
	/// @endcond

	/**
	 * @brief Copy constructor for the Acceptor object
		*
		* Moves the underlying socket handle and port from the source object and invalidates the source object
	 */
	Acceptor(Acceptor&& other) noexcept;

	/**
	 * @brief Copy operator for the Acceptor object
		*
		* Closes any open sockets, moves the socket handle and port from the source object, and invalidates the source object
	 */
	Acceptor& operator=(Acceptor&& other) noexcept;

	/**
	 * @brief Binds the socket handle to the host machine's address
		*
		* Handles getaddrinfo() in passive mode and makes a call to the POSIX bind. See "$ man 2 bind"
	 */
	void bind() const;

	/**
	 * @brief Starts listening on the socket for incoming connections
	 */
	void listen() const;

	/**
	 * @brief Accepts incoming connections synchronously.
		*
		* Makes a call to the POSIX accept and creates a shared pointer to a tcp::Connection object with the resulting file descriptor
		*
		* @returns A shared pointer to a tcp::Connection object whose socket handle is the file descriptor obtained from POSIX accept
	 */
	Connection::connection_ptr accept() const;
private:
	int32_t listening_socket_fd_; /**< The underlying listening socket */
	std::string port_; /**< Port to listen on */
};
}
