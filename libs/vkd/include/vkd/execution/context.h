#pragma once
#include "../core.h"
#include "excecution.h"
#include <cstdint>
#include <cassert>
#include <shared_mutex>



namespace vkd::exec {


	class ThreadType {

		enum  ThreadTypeE :std::uint32_t {
			MianThread = 1,
			Compute = 2,
			Graphics = 3,
			Custom = 0xFF
		};

	public:
		DLL_API static ThreadType MainThread;
		DLL_API static ThreadType ComputeThread;
		DLL_API static ThreadType GraphicsThread;

		static ThreadType custom(std::uint32_t offset) {
			return ThreadType(static_cast<std::uint32_t>(ThreadTypeE::Custom) + offset);
		}

		bool operator==(const ThreadType& right)  const {
			return type_ == right.type_;
		}


		std::uint32_t value() const {
			return type_;
		}



	private:

		ThreadType(std::uint32_t type)
			:type_(type) {
		}

		std::uint32_t type_;

	};
}

namespace std {
	template <>
	struct hash<vkd::exec::ThreadType> {
		inline
			std::size_t  operator()(const vkd::exec::ThreadType& type) const noexcept {
			return std::hash<std::uint32_t>()(type.value());
		}
	};
}

namespace vkd::exec {

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


