#pragma once

#include <system_error>

namespace netpp_error {
	/**
	 * @brief Enumeration representing the different error codes handled by FailedAllResultsErrorCategory
	 */
	enum FailedAllResults {
		BindFailed,
		ConnectFailed
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
		 * @brief Separate error category for failed system-calls for all results of a query list
		 */
		class FailedAllResultsCategory : public std::error_category {
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
	 * @brief Throws an exception with failed_results_error category
		*
		* @param code The given error results from an iterative system-call
		* @param what_arg The explanation of the error
	 */
	void throw_failed_on_all_results_error(FailedAllResults code, std::string what_arg);
} // namespace netpp_error
