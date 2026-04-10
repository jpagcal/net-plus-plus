#include "../include/tcp.hpp"
#include <cerrno>
#include <event2/event.h>
#include <sys/_types/_ssize_t.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <sys/syslimits.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <cstddef>
#include <fcntl.h>
#include "../include/networking.hpp"
#include "../include/resolver.hpp"
#include "../include/error.hpp"

namespace tcp {
Connection::Connection(Connection &&other) noexcept :
	socket_fd_{ other.socket_fd_ },
	io_context_{ std::move(other.io_context_) },
	event_{ std::move(other.event_) }
{
		other.socket_fd_ = networking::invalid_values::invalid_socket_fd;
}

Connection& Connection::operator=(Connection&& other) noexcept {
	if (socket_fd_ != networking::invalid_values::invalid_socket_fd) { // same as equality
		close(socket_fd_);
	}


	socket_fd_ = other.socket_fd_;
	io_context_ = std::move(other.io_context_);
	event_ = std::move(other.event_);
	other.socket_fd_ = networking::invalid_values::invalid_socket_fd;

	return *this;
}

Connection::connection_ptr Connection::create(int32_t socket_fd, async::IOContext::io_context_ptr io_context = nullptr) {
	return Connection::connection_ptr{ new Connection(socket_fd, io_context) };
}


void Connection::set_nonblocking() {
	if (!io_context_) {
		netpp_error::throw_library_error(
			netpp_error::LibraryError::MissingAsyncContext,
		 	"Missing IO Context"
		);
	}

	int32_t status = fcntl(socket_fd_, F_SETFL, O_NONBLOCK);

	if (status == -1) {
		netpp_error::throw_system_error("System-level fcntl() failed");
	}

	event_ = async::event_ptr(
		bufferevent_socket_new(
			io_context_->c_base(),
			socket_fd_,
			0
		),
		bufferevent_free
	);

	bufferevent_enable(event_.get(), EV_READ);
}

bool Connection::is_nonblocking() {
	int32_t flags = fcntl(socket_fd_, F_GETFL);

	if (flags & O_NONBLOCK) return true;
	return false;
}

void Connection::send_async(std::string_view msg, std::function<void()> callback) {
	auto event = event_.get();

	auto ctx = new async::socket::MessageInfo{};
	ctx->msg = msg;
	ctx->num_bytes = msg.length();
	ctx->callback = callback;

	send_header(event, ctx);
}

void Connection::recv_async(std::vector<std::byte> buf, std::function<void()> callback) {
	auto event = event_.get();

	bufferevent_disable(event, EV_READ);
	auto ctx = new async::socket::MessageInfo{};
	ctx->callback = callback;

	bufferevent_setcb(event, async::socket::drain_msg, nullptr, nullptr, ctx);
	bufferevent_setwatermark(event, EV_READ, tcp::Connection::header_size, 0);

	bufferevent_enable(event, EV_READ);
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

		if (sent == -1) {
			netpp_error::throw_system_error("System level send() failed while sending header");
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

		if (sent == -1) {
			netpp_error::throw_system_error("System level send() failed while sending body");
		}

		bytes_sent += sent;
	}
}

void Connection::recv_sync(std::string &buf) {
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

		if (read == -1) {
			netpp_error::throw_system_error("System level recv() failed while reading body");
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

		if (read == -1) {
			netpp_error::throw_system_error("System level recv() failed while reading body");
		}

		bytes_read += read;
	}
}

Acceptor::Acceptor(std::string port, int32_t domain) :
	listening_socket_fd_{
		socket(domain, networking::socket_type::tcp, 0)
	},
	port_{ port } {
		if (listening_socket_fd_ == -1) {
			netpp_error::throw_system_error("System-level socket() call failed in constructor");
		}

		int value{1};
		if ((setsockopt(listening_socket_fd_, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value))) == -1) {
			netpp_error::throw_system_error("System-level setsockopt() failed in constructor");
		}
	}

Acceptor::Acceptor(Acceptor&& other) noexcept :
	listening_socket_fd_{ other.listening_socket_fd_ },
	port_{ other.port_ } {
		other.listening_socket_fd_ = networking::invalid_values::invalid_socket_fd;
		other.port_ = nullptr;
}

Acceptor &Acceptor::operator=(Acceptor &&other) noexcept {
	if (listening_socket_fd_ != networking::invalid_values::invalid_socket_fd) {
		close(listening_socket_fd_);
	}

	listening_socket_fd_ = other.listening_socket_fd_;
	port_ = other.port_;
	other.listening_socket_fd_ = networking::invalid_values::invalid_socket_fd;
	other.port_ = nullptr;

	return *this;
}

Acceptor::~Acceptor() {
	close(listening_socket_fd_);
}

void Acceptor::bind() const {
	conn_resolver::ResolverHints hints{};

	hints.endpoint_type = networking::socket_type::tcp;
	hints.ip_domain = networking::domain::unspecified_domain;
	hints.flags = networking::flags::binding_socket;
	addrinfo c_hints{ conn_resolver::craft_resolver_hints(hints) }, *res;

	int32_t gai_status{ getaddrinfo(nullptr, port_.data(), &c_hints, &res) };

	if (gai_status != 0) {
		netpp_error::throw_gai_error(gai_status, "getaddrinfo() failed");
	}

	auto cur{ res };
	int32_t bind_status{ -1 };

	while (bind_status == -1 and cur != nullptr) {
		bind_status = ::bind(listening_socket_fd_, cur->ai_addr, cur->ai_addrlen);
		cur = cur->ai_next;
	}

	if (bind_status == -1) {
		// out of range
		netpp_error::throw_library_error(
			netpp_error::LibraryError::BindFailed,
			"System-level bind() failed for all results in the address list"
		);
		return;
	}

	freeaddrinfo(res);
}

void Acceptor::listen() const {
	if (::listen(listening_socket_fd_, SOMAXCONN) == -1) {
		netpp_error::throw_system_error("listen() failed");
	}
}

Connection::connection_ptr Acceptor::accept() const {
	int32_t connection_socket;

	if ((connection_socket = ::accept(listening_socket_fd_, nullptr, nullptr)) == -1) {
		netpp_error::throw_system_error("accept() failed");
	}

	Connection::connection_ptr conn{ Connection::create(connection_socket) };

	return conn;
}

} // namespace tcp
