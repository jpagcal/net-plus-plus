#include <event2/event.h>
#include <memory>

namespace async {
class IOContext {
	public:
	IOContext() :
		base_{
			std::make_unique<event_base *>(event_base_new())
		} {}

	event_base *c_base() const;

	private:
		std::unique_ptr<event_base *> base_;
};
} // namespace event
