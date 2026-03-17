#include "../include/tcp.hh"
#include <sys/_types/_ssize_t.h>
#include <sys/fcntl.h>
#include <sys/syslimits.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <cstddef>
#include <fcntl.h>
#include "../include/networking.hpp"

namespace tcp {
Connection::Connection(Connection &&other) noexcept : socket_fd_{ other.socket_fd_ } {
	other.socket_fd_ = networking::invalid_values::invalid_socket_fd;
}

Connection& Connection::operator=(Connection&& other) noexcept {
	if (socket_fd_ != networking::invalid_values::invalid_socket_fd) { // same as equality
		close(socket_fd_);
		socket_fd_ = other.socket_fd_;
		other.socket_fd_ = networking::invalid_values::invalid_socket_fd;
	}

	return *this;
}

Connection::connection_ptr Connection::create(int32_t socket_fd) {
	return Connection::connection_ptr{ new Connection(socket_fd) };
}


void Connection::set_nonblocking() {
	int32_t status = fcntl(socket_fd_, F_SETFL, O_NONBLOCK);

	if (status == -1) {
		//TODO: error handling here
	}
}

bool Connection::is_nonblocking() {
	int32_t flags = fcntl(socket_fd_, F_GETFL);

	if (flags & O_NONBLOCK) return true;
	return false;
}

void Connection::send_async(std::string_view msg, std::function<void()> callback) {
}

void recv_async(std::vector<std::byte> buf, std::function<void()> callback) {
}

void Connection::send_sync(std::string_view msg) {
	Connection::length_header msg_size(
		htonl(msg.length())
	);

	size_t bytes_sent{};

	// send the fixed-length header
	while (bytes_sent < Connection::header_size) {
		ssize_t sent{
			send(
				socket_fd_,
				reinterpret_cast<char *>(&msg_size) + bytes_sent,
				Connection::header_size - bytes_sent,
				0
			)
		};

		if (bytes_sent == -1) {
			// TODO: error handling here
		}

		bytes_sent += sent;
	}

	// send the body
	bytes_sent = 0;
	const char *raw_msg{ msg.data() };

	while (bytes_sent < msg.length()) {
		ssize_t sent {
			send(
				socket_fd_,
				raw_msg + bytes_sent,
				msg.length() - bytes_sent,
				0
			)
		};

		if (bytes_sent == -1) {
			// TODO: error handling here
		}

		bytes_sent += sent;
	}
}

void Connection::recv_sync(std::vector<std::byte> &buf) {
	// read the fixed-length header
	Connection::length_header header{};
	size_t bytes_read{};

	// read fixed-length header
	while (bytes_read < Connection::header_size) {
		ssize_t read{
			recv(
				socket_fd_,
				reinterpret_cast<char *>(&header) + bytes_read,
				Connection::header_size - bytes_read,
				0
			)
		};

		if (bytes_read == -1) {
			// TODO: error handling here
		}

		bytes_read += read;
	}

	bytes_read = 0;
	size_t msg_size{ static_cast<size_t>(ntohl(header)) };
	buf.resize(msg_size);

	while (bytes_read < msg_size) {
		ssize_t read{
			recv(
				socket_fd_,
				buf.data() + bytes_read,
				msg_size - bytes_read,
				0
			)
		};

		if (bytes_read == -1) {
			// TODO: error handling here
		}

		bytes_read += read;
	}
}

Acceptor::Acceptor() : listening_socket_fd_{ socket(networking::domain::unspecified_domain, networking::socket_type::tcp, 0) } {}

Acceptor::Acceptor(Acceptor&& other) noexcept : listening_socket_fd_{ other.listening_socket_fd_ } {
	other.listening_socket_fd_ = networking::invalid_values::invalid_socket_fd;
}

Acceptor &Acceptor::operator=(Acceptor &&other) noexcept {
	if (listening_socket_fd_ != networking::invalid_values::invalid_socket_fd) {
		close(listening_socket_fd_);
	}

	listening_socket_fd_ = other.listening_socket_fd_;
	other.listening_socket_fd_ = networking::invalid_values::invalid_socket_fd;

	return *this;
}

Acceptor::~Acceptor() {
	close(listening_socket_fd_);
}

} // namespace tcp
