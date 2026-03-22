#include <gtest/gtest.h>
#include <memory>
#include "../include/resolver.hpp"

class ResolverTest : public testing::Test {
protected:
	ResolverTest(): res_{
		std::make_unique<conn_resolver::Resolver>(conn_resolver::Resolver("google.com", "80"))
	} {};

	std::unique_ptr<conn_resolver::Resolver> res_;
};

TEST_F(ResolverTest, prints_out_query_results) {
	res_->print_results();
}
