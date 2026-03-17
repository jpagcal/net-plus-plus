#pragma once

#include <sys/socket.h>
#include <cstddef>

namespace networking {
inline namespace constants {
namespace invalid_values {
	constexpr int32_t invalid_socket_fd{ -1 };
}
namespace domain {
	constexpr int ipv4 = AF_INET;
	constexpr int ipv6 = AF_INET6;
	constexpr int32_t unspecified_domain = AF_UNSPEC;
} // namespace domain

namespace socket_type {
	constexpr int tcp = SOCK_STREAM;
	constexpr int udp = SOCK_DGRAM;
	constexpr int raw = SOCK_RAW;
} // namespace socket_type
} // namespace constants
} // namespace networking
