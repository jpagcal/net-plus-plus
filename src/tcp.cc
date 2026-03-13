#include "../include/tcp.hh"
#include <sys/_types/_ssize_t.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <cstddef>
#include <fcntl.h>

namespace tcp {
Connection::Connection(Connection &&other) noexcept : socket_fd_{ other.socket_fd_ } {
	other.socket_fd_ = Connection::invalid_socket_fd;
}

Connection& Connection::operator=(Connection&& other) noexcept {
	if (socket_fd_ != Connection::invalid_socket_fd) { // same as equality
		close(socket_fd_);
		socket_fd_ = other.socket_fd_;
		other.socket_fd_ = Connection::invalid_socket_fd;
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

	callback();
}



} // namespace tcp
