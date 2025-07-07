#include"config.hpp"

namespace vkd::rhi {

	


	std::vector<const char*> gVulkanLayers = {
		#ifdef _DEBUG
			"VK_LAYER_KHRONOS_validation",
		#endif
	};

	std::vector<const char*> gVulkanExtensions = {
	   VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef WIN32
		   VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif // WIN32

#ifdef _DEBUG
		   VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif // DEBUG

	};


	std::vector<const char*> gDeviceExtensions = {

	VK_KHR_SWAPCHAIN_EXTENSION_NAME,

	VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,

	VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,

	VK_EXT_MESH_SHADER_EXTENSION_NAME,

	};
}