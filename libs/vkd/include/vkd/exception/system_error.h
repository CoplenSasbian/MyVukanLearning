

#pragma once
#include <stdexcept>
namespace vkd {
	
	class SystemError:public std::runtime_error {
	public:
		SystemError(const std::string& context,void* handle = nullptr, uint32_t errcode = getLastError());
		
		static uint32_t getLastError()noexcept;

	private:

		const uint32_t errorCode_;
	};

}