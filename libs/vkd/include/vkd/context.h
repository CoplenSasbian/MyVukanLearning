#pragma once
#include "core.h"
#include "excecution.h"
#include <cstdint>
#include <cassert>
#include <shared_mutex>

namespace vkd::exec {


	class ThreadType{
		enum  ThreadTypeE :std::uint32_t{
			MianThread = 1,
			Compute = 2,
			Graphics = 3,
			Custom = 0xFF
		};
	public:
		static ThreadType MainThread;
		static ThreadType ComputeThread;
		static ThreadType GraphicsThread;

		static ThreadType Custom(std::uint32_t offset) {
			assert((static_cast<std::uint32_t>(ThreadTypeE::Custom) + offset) >= static_cast<std::uint32_t>(ThreadTypeE::Custom) && "Offset causes underflow or invalid custom value");
			return ThreadType(static_cast<std::uint32_t>(ThreadTypeE::Custom) + offset);
		}

		operator std::uint32_t() const {
			return type_;
		}

		bool operator==(const ThreadType& other) const {
			return type_ == other.type_;
		}

		std::uint32_t value() {
			return type_;
		}



	private:

		ThreadType(std::uint32_t type)
			:type_(type) {
		}

		std::uint32_t type_;

	};


	class Context {

	public:

		DLL_API Context();
		DLL_API ~Context();
		DLL_API void run();
		DLL_API void poll();

		DLL_API Scheduler getScheduler(ThreadType type);

		DLL_API void addCustomThreadLoop(ThreadType type, vkd::exec::SchedulerProvider* loop);

	private:
		vkd::exec::ThreadPoolLoop threadPoolLoop_;
		vkd::exec::ThreadLoop mainThreadLoop;

		//std::jthread graphicsThread_;
		vkd::exec::ThreadLoop graphicsThreadLoop;

		std::shared_mutex threadLoopsMutex_;
		std::unordered_map<ThreadType, vkd::exec::SchedulerProvider*> customThreadLoops_;

	};


}