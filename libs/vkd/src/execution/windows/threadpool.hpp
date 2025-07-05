#pragma once
#ifdef WIN32
#include <vkd/execution/threadpool.h>
#include <Windows.h>
#include <stdexcept>
#include <cassert>

namespace vkd::exec {

		struct ThreadPool::Pimpl {
			PTP_POOL poolId = nullptr;
			TP_CALLBACK_ENVIRON callbackEnv = { 0 };
			PTP_CLEANUP_GROUP cleanupGroup = nullptr;
		};

		ThreadPool::ThreadPool() 
			:__p(std::make_unique<Pimpl>())
		{
			
			void* pool = nullptr;
			::InitializeThreadpoolEnvironment(&(__p->callbackEnv));
			auto poolId = ::CreateThreadpool(nullptr);
			if (poolId == nullptr)
			{
				throw std::runtime_error("Failed to create thread pool");
			}
			__p->poolId = poolId;


			__p->cleanupGroup = ::CreateThreadpoolCleanupGroup();
			if (__p->cleanupGroup == nullptr) {
				throw std::runtime_error("Failed to create thread pool cleanup group");
			}

			::SetThreadpoolCallbackPool(&__p->callbackEnv, __p->poolId);

			SetThreadpoolCallbackCleanupGroup(&__p->callbackEnv,
				__p->cleanupGroup,
				NULL);
		}

		ThreadPool::ThreadPool(ThreadPool&& rhs)noexcept {
			__p.swap(rhs.__p);
		}

		void ThreadPool::setMaxThreads(int maxThreads) {
			assert(__p->poolId != nullptr && "Thread pool handle is null");
			assert(maxThreads > 0 && "Max threads must be greater than 0");

			auto poolPtr = static_cast<PTP_POOL>(__p->poolId);
			::SetThreadpoolThreadMaximum(poolPtr, maxThreads);
		}


		void ThreadPool::addTask(Task* task) {
		
			assert(__p->poolId != nullptr && "Thread pool handle is null");
			assert(task != nullptr && "Task cannot be null");
			auto taskId = ::CreateThreadpoolWork(
				[](PTP_CALLBACK_INSTANCE instance, PVOID context, PTP_WORK work) {
					auto* task = static_cast<Task*>(context);
					task->execute_(task);
					::CloseThreadpoolWork(work);
				},
				task, &__p->callbackEnv);
			if (taskId == nullptr) {
				throw std::runtime_error("Failed to create thread pool task");
			}
			::SubmitThreadpoolWork(taskId);
		}


		ThreadPool::~ThreadPool() {
			if (__p->cleanupGroup) {
				CloseThreadpoolCleanupGroupMembers(__p->cleanupGroup,
					FALSE,
					NULL);
				CloseThreadpoolCleanupGroup(__p->cleanupGroup);

				__p->cleanupGroup = nullptr;
			}
			CloseThreadpool(std::exchange(__p->poolId, nullptr));
		}


		static inline FILETIME computeTime(int ms) {
			FILETIME FileDueTime;
			ULARGE_INTEGER  ulDueTime{
				.QuadPart = (ULONGLONG)-(ms * 10 * 1000 * 1000)
			};

			FileDueTime.dwHighDateTime = ulDueTime.HighPart;
			FileDueTime.dwLowDateTime = ulDueTime.LowPart;
			return FileDueTime;
		}

		void ThreadPool::addDelayTask(DelayTask* task, int ms) {
			if (task->hasCancel())return;
			
			auto timer =::CreateThreadpoolTimer([](PTP_CALLBACK_INSTANCE instance, PVOID context, PTP_TIMER timer) {
				auto* task = static_cast<DelayTask*>(context);
				try {
					task->execute_(task);
					task->flag_ |= DelayTask::Finshed;
				}
				catch (...) {}

				task->param_ = nullptr;
				::CloseThreadpoolTimer(timer);
				
			}, task, &__p->callbackEnv);
			if (timer == nullptr) {
				throw std::runtime_error("DelayTask create error");
			}
			task->param_ = timer;
			auto ft = computeTime(ms);
			::SetThreadpoolTimer(timer,&ft,0,0);
		}

		void ThreadPool::cancelDelayTask(DelayTask* task) {
			auto timer = (PTP_TIMER)std::exchange(task->param_, nullptr);
			if (timer) {
				::SetThreadpoolTimer(timer, nullptr, 0, 0);
				::CloseThreadpoolTimer(timer);
				task->flag_ |= task->Canceled;
			}
		}
		
		void ThreadPool::addIntervalTask(IntervalTask* task, int ms) {
			

			auto timer = ::CreateThreadpoolTimer([](PTP_CALLBACK_INSTANCE instance, PVOID context, PTP_TIMER timer) {
				auto* task = static_cast<IntervalTask*>(context);
				try {
					task->execute_(task);
					task->flag_ |= IntervalTask::Running;
					task->times_++;
				}
				catch (...) {}
			}, task, &__p->callbackEnv);
			if (timer == nullptr) {
				throw std::runtime_error("IntervalTask create error");
			}
			task->param_ = timer;
			
			auto ft = computeTime(ms);
			::SetThreadpoolTimer(timer, &ft, ms, 0);

		}

		void ThreadPool::cancelIntervalTask(IntervalTask* task) {
			cancelDelayTask(task);
		}


	
}

#endif