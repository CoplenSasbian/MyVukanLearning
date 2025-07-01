#pragma once
#include "scheduler.h"

namespace vkd::exec::__detail
{
	class Env {
	public:
			Env(const SchedulerProvider* provider)
				:provider_(provider) {
			}

			bool operator==(const Env& rhs) const noexcept {
				return provider_ == rhs.provider_;
			}

			friend Scheduler tag_invoke(stdexec::get_scheduler_t, const Env& self) noexcept {
				return Scheduler{(self.provider_) };
			}

			friend Scheduler tag_invoke(stdexec::get_completion_scheduler_t<stdexec::set_value_t>, const Env& self) noexcept {
				return Scheduler{ (self.provider_) };
			}

			std::stop_token stop_requested() const noexcept {
				return stop_source_.get_token();
			}


			friend std::stop_token tag_invoke(std::execution::get_stop_token_t, const Env& self) noexcept {
				return self.stop_requested();
			}

			void request_stop() noexcept {
				stop_source_.stop_possible()
					&& stop_source_.request_stop();
			}

			bool operator==(const Env& rhs) const noexcept {
				return provider_ == rhs.provider_;
			}
			
	private:
		std::stop_source stop_source_{};
		const SchedulerProvider* provider_;
	};
}