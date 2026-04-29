#include "../include/io_context.hpp"
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include "../include/tcp.hpp"
#include "../include/error.hpp"
#include <iostream>

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

IOContext::io_context_ptr IOContext::create() {
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
		AsyncContext* context{ static_cast<AsyncContext *>(ctx) };
		evbuffer *in{ bufferevent_get_input(event) };

		while (true) {
			if (evbuffer_get_length(in) < tcp::Connection::header_size) return;

			int32_t length;
			if (evbuffer_copyout(in, &length, tcp::Connection::header_size) < tcp::Connection::header_size) return;
			length = ntohl(length);

			std::string msg{};
			msg.resize(length);

			if (evbuffer_get_length(in) < length + tcp::Connection::header_size) return;
			evbuffer_drain(in, tcp::Connection::header_size);
			if (evbuffer_remove(in, msg.data(), length) == -1) {
				netpp_error::throw_system_error("evbuffer_remove failed:");
			}

			context->recv_callback(msg);
		}
	}

	void on_write(bufferevent *event, void *ctx) {
		return;
	}

	void on_event(bufferevent *event, short events, void *ctx) {
		return;
	};
} // namespace socket
} // namespace async
