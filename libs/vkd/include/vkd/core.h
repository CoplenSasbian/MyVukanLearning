#pragma once
#include "window_export.h"



namespace vkd{

	struct ImMoveable {
		ImMoveable() = default;
		~ImMoveable() = default;
		ImMoveable(ImMoveable&&) = delete;
		ImMoveable& operator=(ImMoveable&&) = delete;
	};

	struct NonCopyable{
		NonCopyable() = default;
		~NonCopyable() = default;
		NonCopyable(const NonCopyable&) = delete;

		ImMoveable& operator=(const ImMoveable&) = delete;
		
	};

	class NonMovableNonCopyable {
	public:
		NonMovableNonCopyable() = default; 
		~NonMovableNonCopyable() = default;

		NonMovableNonCopyable(const NonMovableNonCopyable&) = delete;
		NonMovableNonCopyable& operator=(const NonMovableNonCopyable&) = delete;

		NonMovableNonCopyable(NonMovableNonCopyable&&) = delete;
		NonMovableNonCopyable& operator=(NonMovableNonCopyable&&) = delete;
	};

}