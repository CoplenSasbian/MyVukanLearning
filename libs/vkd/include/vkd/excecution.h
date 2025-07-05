#pragma once
#include "threadpool.h"
#include "execution/stdexec.h"
#include "core.h"
#include "task.h"
namespace vkd::exec::__detail {

	class Scheduler;
	class SchedulerProvider;

	class SenderEnv {
	public:
		SenderEnv(const SchedulerProvider* provider)
			:provider_(provider) {
		}
		inline
		bool operator==(const SenderEnv& rhs) const noexcept {
			return provider_ == rhs.provider_;
		}

		friend Scheduler tag_invoke(stdexec::get_scheduler_t, const SenderEnv& self) noexcept;

		friend Scheduler tag_invoke(stdexec::get_completion_scheduler_t<stdexec::set_value_t>, const SenderEnv& self) noexcept;
		inline
		std::stop_token stop_requested() const noexcept {
			return stop_source_.get_token();
		}

		inline
		friend std::stop_token tag_invoke(std::execution::get_stop_token_t, const SenderEnv& self) noexcept {
			return self.stop_requested();
		}
		inline
		void request_stop() noexcept {
			stop_source_.stop_possible()
				&& stop_source_.request_stop();
		}



	private:
		std::stop_source stop_source_{};
		const SchedulerProvider* provider_;
	};

	template<stdexec::receiver R>
	class OperationState :public Task {
	public:
		OperationState(R&& receiver, const SchedulerProvider* provider)
			: receiver_(std::forward<R>(receiver)), provider_(provider) {
			Task::execute_ = &OperationState::execute_;
		}
		void start() const noexcept;
		friend void tag_invoke(stdexec::start_t, OperationState& self) noexcept {
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

		const SchedulerProvider* provider_;
		R receiver_;
	};

	class Sender :public std::execution::sender_t {
	public:
		using completion_signatures = std::execution::completion_signatures<
			std::execution::set_value_t(),
			std::execution::set_error_t(std::exception_ptr),
			std::execution::set_stopped_t()>;

		~Sender() {}

		Sender(const SchedulerProvider* schedulerProvider)
			:schedulerProvider_(schedulerProvider) {
		}

		Sender(Sender&& r) noexcept {
			schedulerProvider_ = std::exchange(r.schedulerProvider_, nullptr);
		}
		Sender(const Sender& r)
			:schedulerProvider_(r.schedulerProvider_) {
		}

		inline
		SenderEnv get_env() const noexcept {
			return { schedulerProvider_ };
		}
		inline
		friend  SenderEnv tag_invoke(std::execution::get_env_t, const Sender& self)noexcept {
			return self.get_env();
		}

		template<std::execution::receiver R>
		friend OperationState<R> tag_invoke(std::execution::connect_t, const Sender& self, R&& receiver) {
			return { std::forward<R>(receiver) ,self.schedulerProvider_ };
		}

	private:
		const SchedulerProvider* schedulerProvider_;
	};

	class SchedulerProvider {
	public:
		Scheduler get_scheduler();
		friend class Scheduler;
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
		inline
		friend Sender schedule(const Scheduler& self) {
			return { self.provider_ };
		}
		inline
		friend Sender tag_invoke(std::execution::schedule_t, const Scheduler& self) noexcept {
			return Sender{ self.provider_ };
		}


	private:
		const SchedulerProvider* provider_;
	};
	inline
	Scheduler SchedulerProvider::get_scheduler() {
		return Scheduler{ this };
	}

	template<std::execution::receiver R>
	void OperationState<R>::start() const noexcept {
		provider_->pushTask(provider_,(Task*)this);
	}
	inline
	Scheduler tag_invoke(stdexec::get_scheduler_t, const SenderEnv& self) noexcept{
		return Scheduler{ self.provider_ };
	}
	inline
	Scheduler tag_invoke(stdexec::get_completion_scheduler_t<stdexec::set_value_t>, const SenderEnv& self) noexcept {
		return Scheduler{ self.provider_ };
	}

	

	

	class ThreadRunLoop :public SchedulerProvider {

	public:
		DLL_API ThreadRunLoop();
		DLL_API void run();
		DLL_API void poll();

		DLL_API ~ThreadRunLoop();
	private:

		DLL_API static void _push(const SchedulerProvider* self, Task* task);

		bool stop_ = false;
		std::mutex mutex_;
		std::condition_variable cv_;
		TaskQueue queue_;
	};


	class ThreadPoolRunLoop :public SchedulerProvider {
	public:


		DLL_API ThreadPoolRunLoop();
		DLL_API  ~ThreadPoolRunLoop();

		ThreadPool& getThreadPool() {
			return pool_;
		}

	private:
		DLL_API static void _push(const SchedulerProvider* self, Task* task);
		ThreadPool pool_;
	};




}

namespace vkd::exec {
	using ThreadLoop = vkd::exec::__detail::ThreadRunLoop;
	using ThreadPoolLoop = vkd::exec::__detail::ThreadPoolRunLoop;
	using Scheduler = vkd::exec::__detail::Scheduler;
	using SchedulerProvider = vkd::exec::__detail::SchedulerProvider;
}