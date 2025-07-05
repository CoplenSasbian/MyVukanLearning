#pragma once
#include"../core.h"
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
		
		DelayTask() :Task{}, flag_{} {};
		
		inline
		bool hasFinshed() const {
			return flag_ & Finshed;
		};

		inline
		bool hasCancel() const{
			return flag_ & Canceled;
		
		}
		
	private:
		uint8_t flag_ ;
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

	class DLL_API ThreadPool {
		class Pimpl;
	
	public:
		 ThreadPool();
		 ThreadPool(ThreadPool&&) noexcept;
		 void setMaxThreads(int count) ;
		 void addTask(Task* task);

		 void addDelayTask(DelayTask* task,int ms);

		 void cancelDelayTask(DelayTask* task);

		 void addIntervalTask(IntervalTask* task, int ms);

		 void cancelIntervalTask(IntervalTask* task);

		 ~ThreadPool();
	private:
		std::unique_ptr<Pimpl> __p;
	
	};

}