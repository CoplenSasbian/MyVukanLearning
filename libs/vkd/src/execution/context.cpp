#include<vkd/execution/context.h>
#include <format>

namespace vkd::exec
{
	ThreadType ThreadType::MainThread{ std::to_underlying(ThreadTypeE::MianThread)};
	ThreadType ThreadType::ComputeThread{ std::to_underlying(ThreadTypeE::Compute) };
	ThreadType ThreadType::GraphicsThread{ std::to_underlying(ThreadTypeE::Graphics) };


	Context::Context()
		:threadPoolLoop_{}, mainThreadLoop{}, graphicsThreadPool_{} {
	
	}

	Context::~Context() {
	}

	void Context::run() {
		mainThreadLoop.run();
	}

	void Context::poll() {
		mainThreadLoop.poll();
	}

	Scheduler Context::getScheduler(ThreadType type) {
		if (type == ThreadType::MainThread) {
			return mainThreadLoop.get_scheduler();
		}
		else if (type == ThreadType::GraphicsThread) {
			return 		graphicsThreadPool_.get_scheduler();
		}
		else if (type == ThreadType::ComputeThread) {
			return threadPoolLoop_.get_scheduler();
		}
		else {
			std::shared_lock lock{ threadLoopsMutex_ };
			auto it = customThreadLoops_.find(type);
			if (it == customThreadLoops_.end())
			{
				throw std::out_of_range(std::format("Thread type {}",type.value()));
			}
			return it->second->get_scheduler();
		}
	}
	void Context::addCustomThreadLoop(ThreadType type, SchedulerProvider* loop) {
		if( (ThreadType::ComputeThread == type)||
			(ThreadType::MainThread == type)||
			(ThreadType::GraphicsThread == type)
			) {
			throw std::invalid_argument("Cannot add custom thread loop for predefined thread types");
		}

		if (loop == nullptr) {
			throw std::invalid_argument("Loop cannot be null");
		}

		std::lock_guard lock{ threadLoopsMutex_ };
		customThreadLoops_.emplace(type, loop);
		
	}
}
