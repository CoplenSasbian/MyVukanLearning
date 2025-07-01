#pragma once
#include "core/core.h"
#include "../task.h"
namespace vkd::exec::__detail {
	class SchedulerProvider;

	template<stdexec::receiver R>
	class OperationState:public Task  {
	public:
		OperationState(R&& receiver, const SchedulerProvider* provider)
		: receiver_(std::forward<R>(receiver)), provider_(provider) {
			Task::execute_ = &OperationState::execute_;
		}
		

		void start() const noexcept {
			provider_->pushTask((Task*)this);
		}

		friend void tag_invoke(stdexec::start_t, const OperationState& self) noexcept {
			self.start();
		}

		
	private:

		static void execute_(Task* task) {
			auto* self = static_cast<OperationState*>(task);
			try {

				if (stdexec::get_stop_token(stdexec::get_env(self->receiver_)).stop_requested()) {
					stdexec::set_stopped(std::move(self->receiver_));
					return;
				}
				stdexec::set_value(std::move(self->receiver_));
			}
			catch (...) {
				stdexec::set_error(self->receiver_, std::current_exception());
			}
		}

		SchedulerProvider * provider_;
		R receiver_;
	}
}