#pragma once
#include"core.h"
#include <memory>
#include "task.h"


namespace vkd::exec {
	
	class DelayTask :public Task{
	public:

		enum Flag :uint8_t {
			Finshed = 1,
			Canceled = 0b10,
		};

		~DelayTask()  {};
		DelayTask() :Task{} {};
		
		bool hasFinshed() const {
			return flag_ & Finshed;
		};

		bool hasCancel() const{
			return flag_ & Canceled;
		
		}
		
	private:
		uint8_t flag_;
		friend class ThreadPool;
	};

	class IntervalTask :public DelayTask {
	public:
		enum Flag :uint8_t {
			Running = 0b100
		};

		uint32_t times() const{
			return times_;
		};
	private:
		uint32_t times_ = 0;
		friend class ThreadPool;
	};

	class ThreadPool {
		class Pimpl;
	
	public:
		DLL_API ThreadPool();
		DLL_API ThreadPool(ThreadPool&&) noexcept;
		DLL_API void setMaxThreads(int count) ;
		DLL_API void addTask(Task* task);

		DLL_API void addDelayTask(DelayTask* task,int ms);

		DLL_API void cancelDelayTask(DelayTask* task);

		DLL_API void addIntervalTask(IntervalTask* task, int ms);

		DLL_API void cancelIntervalTask(IntervalTask* task);

		DLL_API ~ThreadPool();
	private:
		std::unique_ptr<Pimpl> __p;
	
	};

}