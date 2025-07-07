#pragma once


#ifdef WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif // WIN32


#include <vulkan/vulkan.hpp>
#ifdef WIN32
#include<Windows.h>
#include<vulkan/vulkan_win32.h>
#endif // WIN32



namespace vkd::rhi {
	class DeviceProvider {
	public:
		DeviceProvider(DeviceProvider* device) :device_(device->device_) {}
		DeviceProvider(vk::Device* device) :device_(device) {}
		DeviceProvider(std::nullptr_t) {}
		vk::Device* device_;
	};

	extern std::vector<const char*> gVulkanLayers; 
	extern std::vector<const char*> gVulkanExtensions;
	extern std::vector<const char*> gDeviceExtensions ;
}