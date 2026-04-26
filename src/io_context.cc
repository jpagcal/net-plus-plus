#include "../include/io_context.hpp"
#include "../include/tcp.hpp"
#include <event2/bufferevent.h>

namespace async {
IOContext::IOContext() : base_{
	std::unique_ptr<event_base, decltype(&event_base_free)>{event_base_new(), event_base_free}
} {}

IOContext::IOContext(IOContext &&other) noexcept :
	base_{std::move(other.base_)} {}

IOContext &IOContext::operator=(IOContext&& other) noexcept {
	if (base_ != nullptr) {
		event_base_free(base_.get());
		base_ = nullptr;
	}

	base_ = std::move(other.base_);
	other.base_ = nullptr;

	return *this;
}

IOContext::io_context_ptr create() {
	return IOContext::io_context_ptr{ new IOContext() };
}

event_base *IOContext::c_base() const {
	return base_.get();
}

void IOContext::run() const {
	event_base_dispatch(base_.get());
}

namespace socket {
	void set_recv_callback(RecvCallback recv_callback, AsyncContextPtr context) {
		context->recv_callback = recv_callback;
	}

	void on_read(bufferevent *event, void *ctx) {
		// get the context
		AsyncContextPtr context{ static_cast<AsyncContext *>(ctx) };
		//get the event buffer
	}

	void on_write() {

	}
} // namespace socket
} // namespace async
