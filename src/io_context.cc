#include "../include/io_context.hpp"

namespace async {
IOContext::IOContext() : base_{
	std::unique_ptr<event_base>{event_base_new()}
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

IOContext::~IOContext() {
	event_base_free(base_.get());
}

event_base *IOContext::c_base() const {
	return base_.get();
}

void IOContext::run() const {
	event_base_dispatch(base_.get());
}
} // namespace async
