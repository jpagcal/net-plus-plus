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

		const char *LibraryErrorCategory::name() const noexcept {
			return "FailedAllResultsCategory";
		}

		std::string LibraryErrorCategory::message(int ev) const {
			switch (ev) {
				case LibraryError::BindFailed:
					return "Check your network configurations";
				case LibraryError::ConnectFailed:
					return "Check hostname, service, and query results";
				case LibraryError::MissingAsyncContext:
					return "Specify an io_context";
				default:
					return "";
			}
		}

		const LibraryErrorCategory library_error{};
		const GAIErrorCategory gai_error{};
	}

	void throw_gai_error(int &ev, std::string what_arg) {
		throw std::system_error(ev, gai_error, std::move(what_arg));
	}

	void throw_results_exhaustion_error(LibraryError code, std::string what_arg) {
		throw std::system_error(code, library_error, std::move(what_arg));
	}
} // namespace netpp_error
