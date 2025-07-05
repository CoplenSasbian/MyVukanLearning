#include <vkd/execution/task.h>
#include <utility>

namespace vkd::exec {

	TaskQueue::TaskQueue() {}
	void TaskQueue::push(Task* task)
	{
		task->next_ = &head_;
		head_.tail_ = head_.tail_->next_ = task;
	}


	Task* TaskQueue::pop()
	{
		if (empty()) {
			return nullptr;
		}
		if (head_.tail_ == head_.next_)
			head_.tail_ = &head_;
		return std::exchange(head_.next_, head_.next_->next_);
	}

	bool TaskQueue::empty() const
	{
		return head_.next_ == &head_;
	}

}