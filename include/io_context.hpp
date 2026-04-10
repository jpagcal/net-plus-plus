#pragma once

#include <event2/bufferevent.h>
#include <event2/event.h>
#include <functional>
#include <memory>
#include <string>

namespace async {
using event_ptr = std::unique_ptr<bufferevent, decltype(&bufferevent_free)>;
/**
 * @brief Handles the event loop
 *
 * Uses libevent under the hood to dispatch callbacks upon event
 */
class IOContext : public std::enable_shared_from_this<IOContext>{
	public:
	using io_context_ptr = std::shared_ptr<async::IOContext>;
	/**
	 * @brief IOContext constructor
		*
		* Allocates memory for, and initializes the underlying event base handle
	 */
	IOContext();

	/// @cond HIDDEN_FROM_DOCS
	IOContext(const IOContext&) = delete;
	const IOContext& operator=(const IOContext&) = delete;
	/// @endcond

	/**
	 * @brief IOContext move constructor
		*
		* Acquires base handle from the source IOContext and invalidates base_
		* property of the source
		*
		* @param other The source IOContext
	 */
	IOContext(IOContext &&other) noexcept;

	/**
	 * @brief IOContext move assignment operator
		*
		* Invalidates the current event base handle and acquires the event base
		* handle from the source IOContext
		*
		* @param other The source IOContext
		* @returns A non-const lvalue reference to the acquiring IOContext
	 */
	IOContext& operator=(IOContext &&other) noexcept;

	/**
	 * @brief Creates an instance of IOContext wrapped in shared_ptr
		*
		* @returns The shared_ptr to the new IOContext instance
	 */
	io_context_ptr create();

	/**
	 * @brief Gives a pointer to the event base struct for libevent calls
		*
		* @returns A non-owning pointer to the underlying handle
	 */
	event_base *c_base() const;

	/**
	 * @brief Runs the event loop
	 */
	void run() const;

	private:
		std::unique_ptr<event_base, decltype(&event_base_free)> base_; /**< An owning pointer to the event base handle */
};


namespace socket {
	/**
	 * @brief Context to pass into callback functions
	 */
	struct MessageInfo {
		int num_bytes; /**< number of bytes contained in body */
		std::string msg; /**< Dynamically allocated msg placeholder */
		std::function<void()> callback;
	};

	/**
	 * @brief Triggered callback when there is at least 8 bytes in the bufferevent's input buffer
		*
		* @param bufferevent A raw non-owning ptr to the bufferevent
		* @param ctx A non-owning pointer to a ReadHeaderInfo struct. Contains a ref to the message
	 */
	void drain_msg(bufferevent *event, void *ctx);

	/**
	 * Triggered callback when there is at least *header* bytes in the bufferevent's input buffer
		*
		* @param bufferevent A raw non-owning ptr to the bufferevent
		* @param ctx A non-owning ptr to a ReadBodyInfo struct. Contains the number of bytes in body
	 */
	void drain_body(bufferevent *, void *ctx);

	/**
	 *
	 */
	void send_header(bufferevent *event, void *ctx);
	void send_body(bufferevent *event, void *ctx);
}

namespace acceptor {

}
} // namespace event
