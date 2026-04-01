#include <arpa/inet.h>
#include <gtest/gtest.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "../include/endpoint_info.hpp"

class AddressInfoTest : public testing::Test {
	protected:
		AddressInfoTest() :	c_sockaddr{}, c_addrinfo{} {
			c_sockaddr.sin_family = networking::domain::ipv4;
			c_sockaddr.sin_port = htons(8080);
			c_sockaddr.sin_len = sizeof(sockaddr_in);
			inet_pton(
				networking::domain::ipv4,
				"127.2.1.0",
				&(c_sockaddr.sin_addr)
			);

			c_addrinfo.ai_family = networking::domain::ipv4;
			c_addrinfo.ai_socktype = networking::socket_type::tcp;
			c_addrinfo.ai_addr = reinterpret_cast<sockaddr *>(&c_sockaddr);
			char canon_name[] = "hostname";
			c_addrinfo.ai_canonname = canon_name;
		}

		addrinfo c_addrinfo;
		sockaddr_in c_sockaddr;
};

TEST_F(AddressInfoTest, getter_functions_match) {
	AddressInfo node{ &c_addrinfo };

	// assert on domain
	EXPECT_EQ(networking::domain::ipv4, node.domain());
	// assert on socket type
	EXPECT_EQ(networking::socket_type::tcp, node.socket_type());
	EXPECT_EQ(0, node.protocol());
	EXPECT_EQ(true, node.is_tcp());
	EXPECT_EQ(false, node.is_udp());
}

TEST_F(AddressInfoTest, outputted_c_addrinfo_matches) {
	AddressInfo node{ &c_addrinfo };
	addrinfo node_addrinfo{ node.c_addrinfo() };

	EXPECT_EQ(node_addrinfo.ai_protocol, c_addrinfo.ai_protocol);
	EXPECT_EQ(node_addrinfo.ai_family, c_addrinfo.ai_family);
	EXPECT_EQ(node_addrinfo.ai_flags, 0);
	EXPECT_EQ(node_addrinfo.ai_socktype, c_addrinfo.ai_socktype);
	EXPECT_STREQ(node_addrinfo.ai_canonname, c_addrinfo.ai_canonname);
	EXPECT_EQ(sizeof(sockaddr), node_addrinfo.ai_addrlen);

	int i = memcmp(&c_addrinfo, &node_addrinfo, sizeof(sockaddr));
	EXPECT_EQ(i, 0);
}
