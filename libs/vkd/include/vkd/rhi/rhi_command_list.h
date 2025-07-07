#pragma once
#include"../core.h"
#include <cstdint>
namespace vkd::rhi {

	enum class CommandListType : std::uint8_t {
		Graphics = 0,
		Transfer = 1,
		Compute = 2,
	};
	


	

	class RHICommandList {

	};
}