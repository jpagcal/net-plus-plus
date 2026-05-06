#pragma once

#include <deque>
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
	static io_context_ptr create();

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
	using RecvCallback = std::function<void(std::string)>;
	using SendCallback = std::function<void()>;
	using EventCallback = std::function<void(short)>;

	/**
	 * @brief Context to pass into callback functions
	 */
	struct AsyncContext {
		std::deque<SendCallback> send_callbacks;
		RecvCallback recv_callback;
		EventCallback event_callback;
	};

	using AsyncContextPtr = std::shared_ptr<AsyncContext>;

	/**
	 * Sets the AsyncContext recv_callback
		*
		* @param recv_callback A function returning a string
	 */
	void set_recv_callback(std::function<void(std::string)> recv_callback);

	/**
	 * @brief Triggered callback for when data has arrived in the events input buffer
		*
		* @param event The event handle for the socket
		* @param ctx The async context
	 */
	void on_read(bufferevent *event, void *ctx);

	/**
	 * @brief Triggered callback for when data is drained from the events output buffer
		*
		* @param event The event handle for the socket
		* @ctx The async context
	 */
	void on_write(bufferevent *event, void *ctx);

	/**
	 * @brief Triggered callback for when an event has occured that is not a read or a write
		*
		* @param event The event handle for the socket
		* @param events The event that occured
		* @param ctx The async context
	 */
	void on_event(bufferevent *event, short events, void *ctx);
}

namespace acceptor {

}
} // namespace event
