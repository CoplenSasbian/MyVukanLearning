#pragma once
#include "../core.h"
#include <tbb/concurrent_queue.h>
namespace vkd::exec{
	
	
	struct Task:public ImMoveable {

		union
		{
			Task* tail_ = nullptr;
			void (*execute_)(Task*);
		};
		union 
		{
			// for queue node
			Task* next_ = nullptr;
			void* param_;
		};
		
	};


	class TaskQueue {
	public:

		DLL_API TaskQueue() ;
		DLL_API void push(Task* task);
		DLL_API Task* pop();
		bool empty() const;
	private:
		Task head_{ {},  &head_ ,&head_ };
	};


	
	using TbbConcurrentTaskQueue = tbb::concurrent_queue<Task*>;


}