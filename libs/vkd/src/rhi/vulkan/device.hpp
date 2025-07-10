#pragma once

#include <vkd/rhi/rhi_device.h>
#include "config.hpp"
#include <ranges>
#include <spdlog/spdlog.h>
#include "image.hpp"
namespace vkd::rhi{

    class VKDevice : public RHIDevice, public DeviceProvider {
    public:

        VKDevice() :DeviceProvider(nullptr) {}

        ~VKDevice() {

        }

        bool init(void* nativeHandel, int width, int height) override {
            hWnd_ = (HWND)nativeHandel;
            CreateInstance();
            SelectDevice();
            CreateDevice();
            createSwapchain(width, height);
        }

        exec::task<void> asyncResizeSwapchain(int width, int height)override {
            
        }
        
        uint64_t submitCommands(std::span<RHICommandList*> commands) override {
            
            
        }
   

        vk::UniqueInstance instance_{};
        vk::UniqueDevice uniqueDevice_{};
        vk::PhysicalDevice physicalDevice_ = nullptr;
        vk::UniqueSurfaceKHR surface_{};
        vk::UniqueSwapchainKHR swapchain_{};
        std::vector<std::unique_ptr<VKImage> > swapchainImages_{};
        std::array<std::uint32_t, 3> familyIndexes_;


        HWND hWnd_;
    private:


        void CreateInstance()
        {
            vk::ApplicationInfo appInfo(
                "Vulkan HPP App",
                VK_MAKE_VERSION(1, 0, 0),
                "vkd Engine",
                VK_MAKE_VERSION(1, 0, 0),
                VK_API_VERSION_1_0
            );

            vk::InstanceCreateInfo createInfo(
                {},
                &appInfo,
                gVulkanLayers.size(),
                gVulkanLayers.data(),
                gVulkanExtensions.size(),
                gVulkanExtensions.data()
            );


            instance_ = vk::createInstanceUnique(createInfo);

            spdlog::info("Vulkan instance created");

        }
        void SelectDevice() {
            auto devices = instance_->enumeratePhysicalDevices();
            int maxBaseScore = 0;
            int maxRamScore = 0;
            vk::PhysicalDevice currentDevice = nullptr;
            std::string name;

            for (auto& d : devices) {

                int baseScore = 0;
                int ramScore = 0;



                vk::PhysicalDeviceMeshShaderFeaturesEXT meshShaderFeatures = {};

                vk::PhysicalDeviceAccelerationStructurePropertiesKHR accelerationStructureFeatures{ };

                vk::PhysicalDeviceRayTracingPipelineFeaturesKHR rtPipelineFeatures{};

                vk::PhysicalDeviceFeatures2 deviceFeatures2{};


                accelerationStructureFeatures.pNext = &meshShaderFeatures;
                rtPipelineFeatures.pNext = &accelerationStructureFeatures;
                deviceFeatures2.pNext = &rtPipelineFeatures;


                d.getFeatures2(deviceFeatures2);


                auto props = d.getProperties2();
                if (props.properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
                    baseScore += 1000;

                if (props.properties.deviceType == vk::PhysicalDeviceType::eIntegratedGpu)
                    baseScore += 100;

                if (props.properties.deviceType == vk::PhysicalDeviceType::eVirtualGpu || props.properties.deviceType == vk::PhysicalDeviceType::eCpu)
                    baseScore += 10;

                auto memProps = d.getMemoryProperties2();

                for (uint32_t i = 0; i < memProps.memoryProperties.memoryHeapCount; i++) {
                    if (memProps.memoryProperties.memoryHeaps[i].flags & vk::MemoryHeapFlagBits::eDeviceLocal) {
                        ramScore += static_cast<int>(memProps.memoryProperties.memoryHeaps[i].size / (1024 * 1024 * 1024)) * 200;
                    }
                }
                spdlog::info("Found device:{} {}GB .{} {}",
                    props.properties.deviceName.data(),
                    ramScore / 200,
                    meshShaderFeatures.meshShader ? "Mesh Shader." : "",
                    rtPipelineFeatures.rayTracingPipeline ? "Ray tracing." : "");


                if (!meshShaderFeatures.meshShader || !rtPipelineFeatures.rayTracingPipeline)
                    continue;



                if (baseScore >= maxBaseScore) {
                    maxBaseScore = baseScore;
                    if (ramScore > maxRamScore) {
                        maxRamScore = ramScore;
                        currentDevice = d;
                        name = props.properties.deviceName.data();
                    }
                }

            }
            spdlog::info("Select device:{} {}GB", name, maxRamScore / 200);
            physicalDevice_ = currentDevice;


        }

        void CreateDevice() {
            auto queueFamilies = physicalDevice_.getQueueFamilyProperties2();
            for (uint32_t i = 0; i < queueFamilies.size(); i++) {
                auto flag = queueFamilies[i].queueFamilyProperties.queueFlags;
                if (flag & vk::QueueFlagBits::eGraphics) {
                    familyIndexes_[std::to_underlying(CommandListType::Graphics)] = i;
                    spdlog::info("Graphycs queue family index", i);
                    continue;
                }
                if (flag & vk::QueueFlagBits::eTransfer) {
                    familyIndexes_[std::to_underlying(CommandListType::Transfer)] = i;
                    spdlog::info("Transfer queue family index", i);
                    continue;
                }
                if (flag & vk::QueueFlagBits::eCompute) {
                    familyIndexes_[std::to_underlying(CommandListType::Compute)] = i;
                    spdlog::info("Compute queue family index", i);
                    continue;
                }

            }
            if (!familyIndexes_[std::to_underlying(CommandListType::Graphics)]) {
                throw std::runtime_error("No graphics queue family found");
            }
            if (!familyIndexes_[std::to_underlying(CommandListType::Transfer)]) {
                throw std::runtime_error("No transfer queue family found");
            }
            if (!familyIndexes_[std::to_underlying(CommandListType::Compute)]) {
                throw std::runtime_error("No compute queue family found");
            }

            float queuePriority = 1;

            std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos = {
                {{},familyIndexes_[std::to_underlying(CommandListType::Graphics)], 1, &queuePriority},
                {{},familyIndexes_[std::to_underlying(CommandListType::Transfer)], 1, &queuePriority},
                { {},familyIndexes_[std::to_underlying(CommandListType::Compute)], 1,&queuePriority }
            };



            vk::DeviceCreateInfo deviceCreateInfo(
                {},
                queueCreateInfos.size(), queueCreateInfos.data(),
                0, nullptr,
                static_cast<uint32_t>(gDeviceExtensions.size()), gDeviceExtensions.data()
            );

            uniqueDevice_ = physicalDevice_.createDeviceUnique(deviceCreateInfo);

            device_ = uniqueDevice_.operator->();

            spdlog::info("Device created.");

        }
        void  createSwapchain(int width, int height) {

            vk::Win32SurfaceCreateInfoKHR surfaceCreateInfo{
            {},
            GetModuleHandleW(nullptr),
            reinterpret_cast<HWND>(hWnd_)
            };

            surface_ = instance_->createWin32SurfaceKHRUnique(surfaceCreateInfo);

            vk::SwapchainCreateInfoKHR swapchainInfo{
           {},
           *surface_,
           2,
           vk::Format::eB8G8R8A8Unorm,
           vk::ColorSpaceKHR::eSrgbNonlinear,
           {width, height},
           1,
           vk::ImageUsageFlagBits::eColorAttachment
            };
            swapchain_ = device_->createSwapchainKHRUnique(swapchainInfo);
            swapchainImages_ = device_->getSwapchainImagesKHR(*swapchain_)
                | std::views::transform([&](vk::Image& i) {
                return  std::make_unique<VKImage>(this, std::move(i));
                    })
                | std::ranges::to<std::vector>()
                ;
        }


        
        
	};

   

}