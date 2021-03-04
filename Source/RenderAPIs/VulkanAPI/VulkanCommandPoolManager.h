#pragma once

#include "VulkanAPIHeader.h"

#include "VulkanSemaphorePool.h"
#include "RenderAPIs/RenderAPI/Interface/CommandList.h"

namespace cube
{
    namespace rapi
    {
        struct VulkanCommandBuffer
        {
            VkCommandBuffer handle = VK_NULL_HANDLE;
            CommandListType type;
            Uint32 poolIndex;
        };

        class VulkanCommandPoolManager
        {
        public:
            VulkanCommandPoolManager(VulkanDevice& device) : mDevice(device)
            {}
            ~VulkanCommandPoolManager() = default;

            VulkanCommandPoolManager(const VulkanCommandPoolManager& other) = delete;
            VulkanCommandPoolManager& operator=(const VulkanCommandPoolManager* rhs) = delete;

            void Initialize(Uint32 numGraphicsPools, Uint32 numTransferPools, Uint32 numComputePools);
            void Shutdown();

            SPtr<CommandListVk> AllocateCommandList(const CommandListAllocateInfo& allocateInfo);
            VulkanCommandBuffer AllocateCommandBuffer(const CommandListAllocateInfo& allocateInfo);

            void FreeCommandList(SPtr<CommandListVk>& commandList);
            void FreeCommandBuffer(VulkanCommandBuffer& commandBuffer);

        private:
            VulkanDevice& mDevice;

            Vector<VkCommandPool> mGraphicsPools;
            Vector<VkCommandPool> mTransferPools;
            Vector<VkCommandPool> mComputePools;
        };
    } // namespace rapi
} // namespace cube
