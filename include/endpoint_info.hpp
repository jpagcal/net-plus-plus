#pragma once

#include "networking.hpp"
#include <cstdint>
#include <string>
#include <netdb.h>
#include <sys/socket.h>

inline namespace endpoint_info {

class Address {
public:
	Address() :
		raw_address_{},
		ip_domain_{ networking::domain::unspecified_domain } {};

	Address(sockaddr *raw_address);

	Address(std::string &address, std::string &port);

	const sockaddr *c_addr() const;
	int32_t ip_domain() const;
	void print_address() const;

private:
	sockaddr_storage raw_address_;
	int32_t ip_domain_;
};

class AddressInfo {
public:
	AddressInfo() = delete;
	AddressInfo(addrinfo *raw_node) :
	 	address_{ Address(raw_node->ai_addr) },
		ip_domain_{raw_node->ai_family},
		socket_type_{ raw_node->ai_socktype },
		protocol_{ raw_node->ai_protocol },
		canonical_name_{ raw_node->ai_canonname == nullptr ? "No name" : raw_node->ai_canonname} {};


	void print_address_info() const;
	addrinfo c_addrinfo() const;

	int domain() const;
	bool is_udp() const;
	bool is_tcp() const;
	int socket_type() const;
	int protocol() const;
	int create_socket() const;
	int connect() const;


private:
	const Address address_;
	const int32_t ip_domain_;
	const int32_t socket_type_;
	const int32_t protocol_{};
	const std::string canonical_name_;
};

} // namespace endpoint_info
