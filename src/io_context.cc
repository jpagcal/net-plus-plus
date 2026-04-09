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
	void read_msg(bufferevent *event, void *ctx) {
		bufferevent_disable(event, EV_READ);
		auto context{ static_cast<ReadHeaderInfo *>(ctx) };

		ReadBodyInfo *info{ new ReadBodyInfo() };
		info->msg = context->msg;

		int32_t *data;
		bufferevent_read(event, data, tcp::Connection::header_size);
		info->num_bytes = ntohl(*data);

		bufferevent_setcb(
			event,
			read_body,
			nullptr,
			nullptr,
			static_cast<void*>(info)
		);

		bufferevent_setwatermark(
			event,
			EV_READ,
			info->num_bytes,
			0
		);

		bufferevent_enable(event, EV_READ);
	}

	void read_body(bufferevent *event, void *ctx) {
		bufferevent_disable(event, EV_READ);

		auto context{ static_cast<ReadBodyInfo *>(ctx) };
		(context->msg).resize(context->num_bytes);

		bufferevent_read(event, (context->msg).data(), context->num_bytes);

		bufferevent_enable(event, EV_READ);
	}
}
} // namespace async
