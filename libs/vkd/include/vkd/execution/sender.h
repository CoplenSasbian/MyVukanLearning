#pragma once
#include"stdexec.h"
#include "env.h"
#include "operation_state.h"


namespace vkd::exec::__detail
{
	class Sender :std::execution::sender_t{
	public:
		using completion_signatures = stdexec::completion_signatures<
			stdexec::set_value_t(),
			stdexec::set_error_t(std::exception_ptr),
			stdexec::set_stopped_t()>;

		Sender(const SchedulerProvider* schedulerProvider)
			:schedulerProvider_(schedulerProvider) {
		}

		Sender(Sender&& r) noexcept {
			schedulerProvider_ = std::exchange(r.schedulerProvider_, nullptr);
		}

		Sender(const Sender& r)
			:schedulerProvider_(r.schedulerProvider_) {
		}
		~Sender(){}



		Env get_env() const {
			return Env{ schedulerProvider_ };
		}

		friend  Env tag_invoke(stdexec::get_env_t _, const Sender& self)noexcept {
			return self.get_env();
		}

		template<stdexec::receiver R>
		friend OperationState<R> tag_invoke(stdexec::connect_t, const Sender& self, R && receiver) {
			return {  std::forward<R>(receiver) ,self.schedulerProvider_ };
		}
		
		
		
		
	private:
		const SchedulerProvider* schedulerProvider_;
	};

}