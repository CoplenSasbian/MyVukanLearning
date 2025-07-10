#pragma once

#include<vkd/rhi/rhi_image.h>
#include "config.hpp"
namespace vkd::rhi {
	class  VKImage :public RHIImage, public DeviceProvider {
	public:
		VKImage(DeviceProvider* device, vk::Image&& image)
			:DeviceProvider(device), image_(std::move(image_)){
		 
		}
		


		~VKImage() {
			if(image_)
				device_->destroyImage(image_);
		}
		
		
	public:
		vk::Image image_;
	private:

	};
}