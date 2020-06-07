#pragma once

#include "VulkanAPIHeader.h"

#include "VulkanMemoryAllocator.h"

namespace cube
{
    namespace rapi
    {
        enum class GPUType
        {
            Discrete,
            Integrated,
            Virtual,
            CPU,
            Other,
            Unknown
        };

        class VulkanDevice
        {
        public:
            VulkanDevice(VkInstance instance, VkPhysicalDevice gpu);
            ~VulkanDevice();

            GPUType GetGPUType() const { return mType; }
            VkDevice GetDevice() const { return mDevice; }
            VulkanMemoryAllocator GetAllocator() { return mAllocator; }

        private:
            void CreateDevice();

            VkPhysicalDevice mGPU;
            VkPhysicalDeviceProperties mProps;
            VkPhysicalDeviceFeatures mFeatures;
            VkPhysicalDeviceMemoryProperties mMemProps;
            Vector<VkQueueFamilyProperties> mQueueFamilyProps;

            GPUType mType;

            VkDevice mDevice;
            VulkanMemoryAllocator mAllocator;
        };
    } // namespace rapi
} // namespace cube
