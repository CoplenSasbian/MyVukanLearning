#include<vkd/execution/excecution.h>

namespace vkd::exec::__detail
{

	ThreadRunLoop::ThreadRunLoop() {
		pushTask = &ThreadRunLoop::_push;
	}

	void ThreadRunLoop::run() {
		while (!stop_) {
			Task* task;
			if (queue_.try_pop(task)) {
				task->execute_(task);
			}
#if defined(_MSC_VER) || defined(__INTEL_COMPILER)
			_mm_pause();  
#elif defined(__GNUC__) && (__i386__ || __x86_64__)
			__asm__ __volatile__("pause");
#else
			static std::atomic_flag flag = ATOMIC_FLAG_INIT;
			flag.clear(std::memory_order_release); 
#endif
		}
	}

	void ThreadRunLoop::poll() {
		while (!stop_) {
			if(queue_.empty()) return;
			Task* task ;
			if (queue_.try_pop(task)) {
				task->execute_(task);
			}
			else {
				break;
			}
		}
	}

	void ThreadRunLoop::_push(const SchedulerProvider* self, Task* task) {
		auto* loop = const_cast<ThreadRunLoop*>(static_cast<const ThreadRunLoop*>(self));
		loop->queue_.push(task);
	}
	ThreadRunLoop::~ThreadRunLoop() {
		stop_ = true;
	}


	ThreadPoolRunLoop::ThreadPoolRunLoop()
	{
		pushTask = &ThreadPoolRunLoop::_push;
	}
	
	void ThreadPoolRunLoop::_push(const SchedulerProvider* self, Task* task) {
		auto* loop = 
			const_cast<ThreadPoolRunLoop*>(static_cast<const ThreadPoolRunLoop*>(self));
		loop->pool_.addTask(task);
	}

	ThreadPoolRunLoop::~ThreadPoolRunLoop()
	{
		
	
	}
}


