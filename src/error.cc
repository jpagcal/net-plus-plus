#include <iostream>
#include <netdb.h>
#include <string>
#include "../include/error.hpp"

namespace netpp_error {
	void throw_system_error(std::string what_arg) {
		throw std::system_error(errno, std::generic_category(), std::move(what_arg));
	}

	void log_error(std::system_error &e) {
		std::cerr << e.code() << " -------- " << e.what() << '\n';
	}

	namespace {
		const char *GAIErrorCategory::name() const noexcept {
			return "GAIErrorCategory";
		}

		std::string GAIErrorCategory::message(int ev) const {
			return gai_strerror(ev);
		}

		const char *FailedAllResultsCategory::name() const noexcept {
			return "FailedAllResultsCategory";
		}

		std::string FailedAllResultsCategory::message(int ev) const {
			switch (ev) {
				case FailedAllResults::BindFailed:
					return "Check your network configurations";
				case FailedAllResults::ConnectFailed:
					return "Check hostname, service, and query results";
				default:
					return "";
			}
		}

		const FailedAllResultsCategory failed_results_error{};
		const GAIErrorCategory gai_error{};
	}

	void throw_gai_error(int &ev, std::string what_arg) {
		throw std::system_error(ev, gai_error, std::move(what_arg));
	}

	void throw_failed_on_all_results_error(FailedAllResults code, std::string what_arg) {
		throw std::system_error(code, failed_results_error, std::move(what_arg));
	}
} // namespace netpp_error
