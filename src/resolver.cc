#include <cstddef>
#include <cstring>
#include <netdb.h>
#include "../include/resolver.hpp"
#include "../include/error.hpp"

namespace conn_resolver {
addrinfo craft_resolver_hints(ResolverHints &hints) {
	addrinfo raw_hints{};

	raw_hints.ai_flags = hints.flags;
	raw_hints.ai_family = hints.ip_domain;
	raw_hints.ai_socktype = hints.endpoint_type;
	raw_hints.ai_protocol = hints.protocol;

	return raw_hints;
}

Resolver::Resolver(std::string hostname, std::string service) {
	addrinfo *res;

	ResolverHints hints{};
	addrinfo c_hints{ craft_resolver_hints(hints) };

	int32_t getaddrinfo_status{};

	if ((getaddrinfo_status = getaddrinfo(
			hostname.c_str(),
			service.c_str(),
			&c_hints,
			&res
	))) {
		// TODO: error handling here
	}

	conn_resolver::Resolver::QueryResults results{};

	for (addrinfo *cur{ res }; cur->ai_next != nullptr; cur = cur->ai_next) {
		results.emplace_back(AddressInfo(cur));
	}

	// dynamically allocated res linked list not needed at this point
	freeaddrinfo(res);

	query_results_ = results;
}

void Resolver::print_results() const {
	for (const AddressInfo &node : this->query_results_) {
		node.print_address_info();
	}
}

Resolver::QueryResults Resolver::results() const {
	return this->results();
}

Resolver::QueryResults Resolver::udp_nodes() const {
	Resolver::QueryResults udp_nodes{};

	for (const AddressInfo &node : this->query_results_) {
		if (node.is_udp()) {
			udp_nodes.emplace_back(node);
		}
	}

	return udp_nodes;
}

Resolver::QueryResults Resolver::tcp_nodes() const {
	Resolver::QueryResults tcp_nodes{};

	for (const AddressInfo &node : this->query_results_) {
		if (node.is_tcp()) {
			tcp_nodes.emplace_back(node);
		}
	}

	return tcp_nodes;
}

tcp::Connection::connection_ptr Resolver::try_connect_tcp() {
	auto cur{ tcp_nodes().begin() };
	int socket_fd{ networking::invalid_values::invalid_socket_fd };

	while (cur != tcp_nodes().end() && socket_fd == networking::invalid_values::invalid_socket_fd) {
		socket_fd = cur->connect();
	}

	// if this is the case, connect has failed for all nodes
	if (socket_fd == networking::invalid_values::invalid_socket_fd) {
		// TODO: need to define a new error category here with status codes for end-of-list
	}

	return tcp::Connection::create(socket_fd);
}
}
