#pragma once

#include<vkd/rhi/rhi_image.h>
#include "config.hpp"
namespace vkd::rhi {
	class  VKImage :public RHIImage, public VulkanImpl {
	public:
		VKImage(VulkanImpl* device, vk::Image&& image)
			:VulkanImpl(device), image_(std::move(image_)){
			
		}
		
		
		
		
	public:
		vk::Image image_;
	private:

	};
}