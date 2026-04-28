#pragma once

#include <system_error>

namespace netpp_error {
	/**
	 * @brief Enumeration representing the different error codes handled by LibraryErrorCategory
	 */
	enum LibraryError {
		BindFailed,
		ConnectFailed,
		MissingAsyncContext
	};

	/**
	 * @brief Throws an exception with std::generic category()
		*
		* Constructs a system error with errno, std::generic_category, and the passed what_arg, and throws the error
		*
		* @param what_arg An explanatory string passed as a substring to the exception's what()
	 */
	void throw_system_error(std::string what_arg);

	/**
	 * @brief Prints the error to stdout
		*
		* @param e The system error thrown
	 */
	void log_error(std::system_error &e);

	namespace {
		/**
		 * @brief Represents an error category for the getaddrinfo() error codes
		 */
		class GAIErrorCategory : public std::error_category {
			/// @cond HIDDEN_FROM_DOCS
			const char *name() const noexcept override;
			std::string message(int ev) const override;
			/// @endcond
		};

		/**
		 * @brief Separate error category for library errors not related to
			* low-level system calls or getaddrinfo()
		 */
		class LibraryErrorCategory : public std::error_category {
			/// @cond HIDDEN_FROM_DOGS
			const char *name() const noexcept override;
			std::string message(int ev) const override;
			/// @endcond
		};
	}

	/**
	 * @brief Throws an exception with gai_error category
		*
		* @param ev The resulting error code from a getaddrinfo() call
		* @param what_arg The explanation of the error
	 */
	void throw_gai_error(int &ev, std::string what_arg);

	/**
	 * @brief Throws an exception with library_error category
		*
		* @param code The error code pertaining to the library error
		* @param what_arg The explanation of the error
	 */
	void throw_library_error(LibraryError code, std::string what_arg);
} // namespace netpp_error
