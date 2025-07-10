#pragma once
#include"../core.h"
#include "rhi_buffer.h"
#include "rhi_image.h"
#include "rhi_command_list.h"
#include "rhi_graphics_pso.h"
#include <exec/task.hpp>
#include <span>
namespace vkd::rhi {

	
	class RHIDevice {
	public:
		virtual ~RHIDevice() = default;

		virtual bool init(void* nativeHandel,int width,int height) = 0;
		
		virtual void shutdown() = 0;

		virtual RHIBuffer* createResource(const RHIBufferDesc&) = 0;
		
		virtual RHIImage* createImage(const RHIImage&) = 0;

		virtual RHICommandList* createCommandList() = 0;

		virtual uint64_t submitCommands(std::span<RHICommandList*> commands) = 0;

		virtual RHIGraphicsPSO* CreateGraphicsPSO(const GraphicsPipelineStateInitializer& Init) = 0;
		
		virtual exec::task<void> asyncResizeSwapchain(int width, int height) =0 ;

	};

}