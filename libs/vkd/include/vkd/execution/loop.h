#pragma once

#include "core/core.h"
#include "../task.h"
#include "scheduler.h"
#include "../threadpool.h"
namespace vkd::exec::__detail
{


	class ThreadRunLoop :public SchedulerProvider {

	public:
		DLL_API ThreadRunLoop();
		DLL_API void run();
		DLL_API void poll();

		DLL_API ~ThreadRunLoop();
	private:

		DLL_API static void _push(const SchedulerProvider* self,Task* task);
	
		bool stop_ = false;
		std::mutex mutex_;
		std::condition_variable cv_;
		TaskQueue queue_;
	};


	class ThreadPoolRunLoop :public SchedulerProvider {
	public:


		DLL_API ThreadPoolRunLoop();
		DLL_API  ~ThreadPoolRunLoop();

		ThreadPool& getThreadPool()  {
			return pool_;
		}

	private:
		DLL_API static void _push(const SchedulerProvider* self, Task* task);
		ThreadPool pool_;
	};


}

namespace vkd::exec{
	using ThreadLoop = vkd::exec::__detail::ThreadRunLoop;
	using ThreadPoolLoop = vkd::exec::__detail::ThreadPoolRunLoop;
}