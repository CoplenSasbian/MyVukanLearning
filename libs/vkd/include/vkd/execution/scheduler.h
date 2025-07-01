#pragma once
#include "stdexec.h"
#include "sender.h"
namespace vkd::exec::__detail
{
	class Scheduler;

	class SchedulerProvider {
	public:
		Scheduler get_scheduler() {
			return Scheduler{ this };
		};
	protected:
		friend Scheduler;
		void (*pushTask)(const SchedulerProvider*, Task*);
		
	};

	class Scheduler {
	public:
		Scheduler(const SchedulerProvider* provider)
			:provider_(provider) {
		}
		constexpr bool operator==(const Scheduler& other) const noexcept {
			return provider_ == other.provider_;
		}

		friend Sender schedule(const Scheduler& self) {
			return { self.provider_ };
		}

		friend Sender tag_invoke(std::execution::schedule_t, const Scheduler& self) noexcept {
			return Sender{ self.provider_ };
		}


	private:
		const SchedulerProvider* provider_;
	};

	
}

namespace vkd::exec {
	using Scheduler = __detail::Scheduler;
	using SchedulerProvider = __detail::SchedulerProvider;
}