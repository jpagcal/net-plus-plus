#pragma once

#include "endpoint_info.hpp"
#include "networking.hpp"
#include "tcp.hpp"
#include <cstdint>
#include <netdb.h>
#include <string>
#include <vector>

namespace conn_resolver {

/**
 * @brief A structure to represent the hint parameters passed into getaddrinfo
 */
struct ResolverHints {
	int32_t endpoint_type = networking::socket_type::raw; /**< The socket type of the current endpoint */
	int32_t ip_domain = networking::domain::unspecified_domain; /**< The IP domain that the resolver should search */
	int32_t protocol; /**< Limits the returned address structure to a specific protocol */
	int32_t flags; /**< Additional flags. Check 'man getaddrinfo' */
};

/**
 * @brief Creates the raw struct addrinfo out of struct ResolverHints
 *
 * @param hints The resolver hints struct
 */
addrinfo craft_resolver_hints(ResolverHints &hints);

/**
 * @brief The Resolver class
 *
 * Encapsulates getaddrinfo functionality, providing methods to filter, print, and connect to results
 */
class Resolver {
	using QueryResults = std::vector<AddressInfo>;
	using RawResults = addrinfo *;
public:

	/**
	 * @brief The Resolver constructor
	 *
	 * @param hostname the host to connect to
	 * @param service the desired service of the target host
	 */
	Resolver(std::string hostname, std::string service);

	/**
	 * @brief Returns attribute query_results_
	 */
	QueryResults results() const;

	/**
	 * @brief Returns only UDP nodes in query_results_
	 */
	QueryResults udp_nodes() const;

	/**
	 * @brief Returns only TCP nodes in query_results
	 */
	QueryResults tcp_nodes() const;

	/**
	 * @brief Attempts to connect to the target host using protocol TCP
	 *
	 * @returns a TCP connection to the target host
	 */
	tcp::Connection::connection_ptr try_connect_tcp();

	/**
	* @brief Attempts to connect to the target host using protocol UDP
	*
	* @returns a UDP connection to the target host
	*/
	void try_connect_udp(); // TODO: create UDPSession class

	/**
	 * @brief prints query results to stdout
	 */
	void print_results() const;

private:
	QueryResults query_results_; /**< A vector of results filled by the constructor */
};
} // namespace resolver
