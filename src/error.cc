#include <iostream>
#include <netdb.h>
#include <string>
#include <system_error>
#include "../include/error.hpp"

namespace netpp_error {
	void throw_system_error(std::string what_arg) {
		throw std::system_error(errno, std::generic_category(), std::move(what_arg));
	}

	void log_error(std::system_error &e) {
		std::cout << e.code() << "--------" << e.what() << '\n';
	}

	namespace {
		const char *GAIErrorCategory::name() const noexcept {
			return "GAIErrorCategory";
		}

		std::string GAIErrorCategory::message(int ev) const {
			return gai_strerror(ev);
		}

		const GAIErrorCategory gai_error{};
	}

	void throw_gai_error(int &ev, std::string what_arg) {
		throw std::system_error(ev, gai_error, std::move(what_arg));
	}
}
