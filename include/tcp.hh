#pragma once

#include <functional>
#include <memory>
#include <string_view>
#include <sys/socket.h>

namespace tcp {
/**
 * @brief Represents a TCP connection
 *
 * Encapsulates a connected socket, allowing send and receive capabilities, as well as
 * I/O multiplexing using libevent.
 */
class Connection : public std::enable_shared_from_this<Connection> {
	using length_header = int32_t;
	static constexpr int32_t invalid_socket_fd{ -1 };
	static constexpr size_t header_size{ sizeof(length_header )};
	/**
	 * @brief Strong enumeration for socket read state
	 */
	enum class ReadMode {
		reading_header, /**< Socket is currently reading header */
		reading_body /**< Socket is currently reading body */
	};
public:
	using connection_ptr = std::shared_ptr<Connection>;
	/// @cond HIDDEN_FROM_DOCS
	Connection(const Connection&) = delete;
	Connection& operator=(const Connection&) = delete;
	/// @endcond

	/**
	 * @brief The move constructor for the Connection class
	 *
	 * Moves the source socket handle to the current Connection instance, and
	 * invalidates the source instance's socket handle.
	 *
	 * @param other An rvalue reference to the object to steal resources from
	 */
	Connection(Connection&& other) noexcept;
	/**
	 * @brief The move constructor for the Connection class
	 *
	 * Closes the current Connection's socket handle if applicable, moves the source
	 * socket handle to the current connection instance , and invalidates the source
     * instance's socket handle.
	 *
	 * @param other An rvalue reference to the object to steal resources from
	 */
	Connection &operator=(Connection&& other) noexcept;

	/**
	 * @brief Creates a shared ptr to a new Connection instance
		*
		* @param socket_fd The Connected socket file descriptor to pass into the private constructor
		* @returns A shared pointer to the new Connection instance
	 */
	static connection_ptr create(int32_t socket_fd);

	/**
	 * @brief Sets the underlying socket handle as nonblocking
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
		* @param the buffer of bytes to send
	 */
	void send_sync(std::string_view msg);

	/**
	 * @brief Receives a message from the endpoint synchronously
		*
		* Only use when is_nonblocking() returns false. Synchronously receives
		* all bytes into buf.
		*
		* @param The buffer of bytes to receive data
	 */
	void recv_sync(std::vector<std::byte> &buf);
private:
	/// @cond HIDDEN_FROM_DOCS
	Connection(int32_t socket_fd) : socket_fd_{ socket_fd } {};
	/// @endcond

	int32_t socket_fd_; /**< A handle to the connected socket */
	ReadMode read_mode_ = ReadMode::reading_header; /**< The current read mode for the socket */
};

/**
 * @brief Accepts connections and assigns a dedicated TCPConnection object
 *
 * Encapsulates a listening socket
 */
class Acceptor {
public:
	Acceptor();
	~Acceptor();
	Acceptor(const Acceptor&) = delete;
	Acceptor(Acceptor&& other) noexcept;

	Acceptor& operator=(const Acceptor&) = delete;
	Acceptor& operator=(Acceptor&& other) noexcept;
	void bind();
	void listen() const;
	void do_accept() const;
private:
	int32_t listening_socket_fd_;
};
}
