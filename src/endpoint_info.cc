#include "../include/endpoint_info.hpp"
#include "../include/networking.hpp"
#include "../include/error.hpp"
#include <cstdint>
#include <netinet/in.h>
#include <sys/_endian.h>
#include <arpa/inet.h>
#include <iostream>
#include <sys/socket.h>

inline namespace endpoint_info {
	Address::Address(sockaddr *raw_address) {
		// initialize with copies of data
		this->ip_domain_ = raw_address->sa_family;
		this->raw_address_ = *(reinterpret_cast<sockaddr_storage *>(raw_address));
	}

	Address::Address(std::string &address, std::string &port) {
		int32_t ip_domain;
		uint16_t port_num = htons(std::stoi(port));
		auto colon_delim = address.find(":");
		sockaddr_storage raw_address{};

		// depending on whether or not we can find a colon we can identify the address domain
		switch (colon_delim) {
			// then we set the fields on on the storage
			case std::string::npos: { // ipv4
				sockaddr_in *ipv4_raw_address{ reinterpret_cast<sockaddr_in *>(&raw_address) };
				ip_domain = networking::domain::ipv4;

				ipv4_raw_address->sin_family = networking::domain::ipv4;
				ipv4_raw_address->sin_port = port_num;

				in_addr *protocol_address{ &(ipv4_raw_address->sin_addr) };
				inet_pton(
					networking::domain::ipv4,
					address.c_str(),
					reinterpret_cast<void *>(protocol_address)
				);
				break;
			}
			default: { // ipv6
				sockaddr_in6 *ipv6_raw_address { reinterpret_cast<sockaddr_in6 *>(&raw_address) };
				ip_domain = networking::domain::ipv6;

				ipv6_raw_address->sin6_family = networking::domain::ipv6;
				ipv6_raw_address->sin6_port = port_num;

				in6_addr *protocol_address { &(ipv6_raw_address->sin6_addr) };
				inet_pton(
					networking::domain::ipv6,
					address.c_str(),
					reinterpret_cast<void *>(protocol_address)
				);
				break;
			}
		}

		this->raw_address_ = raw_address;
		this->ip_domain_ = ip_domain;
	}

	int32_t Address::ip_domain() const {
		return this->ip_domain_;
	}

	const sockaddr *Address::c_addr() const {
		return reinterpret_cast<const sockaddr *>(
			&(this->raw_address_)
		);
	}

	void Address::print_address() const {
		auto get_port_str{
			[](uint16_t port_num) -> std::string {
				return std::to_string(
					ntohs(port_num)
				);
			}
		};

		char addr_buf[INET6_ADDRSTRLEN]{};
		std::string port;

		switch (this->ip_domain_) {
			case networking::domain::ipv4: {
				auto ipv4_raw_address{
						reinterpret_cast<const sockaddr_in *>(
							&(this->raw_address_)
					)
				};
				inet_ntop(
					networking::domain::ipv4,
					&ipv4_raw_address->sin_addr,
					addr_buf,
					INET6_ADDRSTRLEN
				);

				port = get_port_str(ipv4_raw_address->sin_port);
				break;
			}

			default: {
				auto ipv6_raw_address {
					reinterpret_cast<const sockaddr_in6 *>(
						&(this->raw_address_)
					)
				};
				inet_ntop(
					networking::domain::ipv6,
					&(ipv6_raw_address->sin6_addr),
					addr_buf,
					INET6_ADDRSTRLEN
				);

				port = get_port_str(ipv6_raw_address->sin6_port);
				break;
			}
		}


		std::cout << addr_buf << ":" << port << '\n';
	}

	void AddressInfo::print_address_info() const {
		std::string protocol;
		std::string domain;
		std::string socket_type;
		std::string address;

		std::cout << "IP domain: ";
		switch (this->protocol_) {
			case networking::domain::ipv4:
				std::cout << "IPV4" << std::endl;
				break;
			case networking::domain::ipv6:
				std::cout << "IPV6" << std::endl;
				break;
			default:
				std::cout << "Unspecified" << std::endl;
		}

		std::cout << "Name: " << canonical_name_ << std::endl;
		std::cout << "Address: ";
	 	(this->address_).print_address();

	}

	int AddressInfo::domain() const {
		return this->ip_domain_;
	}

	int AddressInfo::socket_type() const {
		return this->socket_type_;
	}

	int AddressInfo::protocol() const {
		return this->protocol_;
	}

	bool AddressInfo::is_udp() const {
		return (this->socket_type_ == networking::socket_type::udp);
	}

	bool AddressInfo::is_tcp() const {
		return (this->socket_type_ == networking::socket_type::tcp);
	}

	addrinfo AddressInfo::c_addrinfo() const {
		addrinfo c_addrinfo{};

		c_addrinfo.ai_family = this->ip_domain_;
		c_addrinfo.ai_socktype = this->socket_type_;
		c_addrinfo.ai_protocol = this->protocol_;
		c_addrinfo.ai_addrlen = sizeof(sockaddr);
		c_addrinfo.ai_canonname = const_cast<char *>(this->canonical_name_.c_str());

		const sockaddr *const c_addr{ (this->address_).c_addr() };
		c_addrinfo.ai_addr = const_cast<sockaddr *const>(c_addr);

		return c_addrinfo;
	}

	int AddressInfo::create_socket() const {
		int socket_fd;
		if ((socket_fd = socket(ip_domain_, socket_type_, protocol_)) == networking::invalid_values::invalid_socket_fd) {
			netpp_error::throw_system_error("System-level socket() call failed");
		}

		return socket_fd;
	}

	int AddressInfo::connect() const {
		int socket_fd{ -1 };
		try {
			int conn_status;
			addrinfo c_addrinfo{ this->c_addrinfo() };
			socket_fd = create_socket();

			if ((conn_status = ::connect(socket_fd, c_addrinfo.ai_addr, c_addrinfo.ai_addrlen)) == -1) {
				netpp_error::throw_system_error("System-level connect() call failed");
			}
		} catch (std::system_error &e) {
			// we expect reasonably that it will throw for some connect calls, so we just log the error to stderr
			netpp_error::log_error(e);
		}

		return socket_fd;
	}


} // namespace endpoint_info
