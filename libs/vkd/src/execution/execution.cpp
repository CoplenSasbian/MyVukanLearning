#include<vkd/execution/excecution.h>

namespace vkd::exec::__detail
{

	ThreadRunLoop::ThreadRunLoop() {
		pushTask = &ThreadRunLoop::_push;
	}

	void ThreadRunLoop::run() {
		while (!stop_) {
			std::unique_lock lock(mutex_);
			cv_.wait(lock, [&] {return !queue_.empty(); });
			Task* task = queue_.pop();
			lock.unlock();
			if (task) {
				task->execute_(task);
			}
		}
	}

	void ThreadRunLoop::poll() {
		while (!stop_) {
			std::unique_lock lock(mutex_);
			if(queue_.empty()) return;

			Task* task = queue_.pop();
			lock.unlock();

			if (task) {
				task->execute_(task);
			}
		}
	}

	void ThreadRunLoop::_push(const SchedulerProvider* self, Task* task) {
		auto* loop = const_cast<ThreadRunLoop*>(static_cast<const ThreadRunLoop*>(self));

		std::unique_lock lock(loop->mutex_);
		loop->queue_.push(task);
		lock.unlock();
		loop->cv_.notify_one();
	}
	ThreadRunLoop::~ThreadRunLoop() {
		stop_ = true;
		cv_.notify_all();
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


