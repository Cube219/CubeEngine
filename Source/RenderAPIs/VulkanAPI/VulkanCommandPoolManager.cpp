#include "VulkanCommandPoolManager.h"

#include "Interface/CommandListVk.h"
#include "VulkanDebug.h"
#include "VulkanDevice.h"
#include "VulkanQueueManager.h"

#include "Core/Allocator/FrameAllocator.h"
#include "Core/Assertion.h"

namespace cube
{
    namespace rapi
    {
        void VulkanCommandPoolManager::Initialize(Uint32 numGraphicsPools, Uint32 numTransferPools, Uint32 numComputePools)
        {
            VkResult res;

            VkCommandPoolCreateInfo info;
            info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            info.pNext = nullptr;
            info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

            VulkanQueueManager& queueManager = mDevice.GetQueueManager();
            VkCommandPool pool;

            // Graphics pool
            {
                info.queueFamilyIndex = queueManager.GetGraphicsQueueFamilyIndex();

                mGraphicsPools.resize(numGraphicsPools);
                for(Uint32 i = 0; i < numGraphicsPools; ++i) {
                    res = vkCreateCommandPool(mDevice.GetHandle(), &info, nullptr, &pool);
                    CHECK_VK(res, "Failed to create a graphics command pool.");
                    VULKAN_SET_OBJ_NAME(mDevice.GetHandle(), pool, FrameFormat("VkCommandPool for graphics[{}]", i).c_str());

                    mGraphicsPools[i] = pool;
                }
            }

            // Transfer pool
            {
                info.queueFamilyIndex = queueManager.GetTransferQueueFamilyIndex();

                mTransferPools.resize(numTransferPools);
                for(Uint32 i = 0; i < numTransferPools; ++i) {
                    res = vkCreateCommandPool(mDevice.GetHandle(), &info, nullptr, &pool);
                    CHECK_VK(res, "Failed to create a transfer command pool.");
                    VULKAN_SET_OBJ_NAME(mDevice.GetHandle(), pool, FrameFormat("VkCommandPool for transfer[{}]", i).c_str());

                    mTransferPools[i] = pool;
                }
            }

            // Compute pool
            {
                info.queueFamilyIndex = queueManager.GetComputeQueueFamilyIndex();

                mComputePools.resize(numComputePools);
                for(Uint32 i = 0; i < numComputePools; ++i) {
                    res = vkCreateCommandPool(mDevice.GetHandle(), &info, nullptr, &pool);
                    CHECK_VK(res, "Failed to create a compute command pool.");
                    VULKAN_SET_OBJ_NAME(mDevice.GetHandle(), pool, FrameFormat("VkCommandPool for compute[{}]", i).c_str());

                    mComputePools[i] = pool;
                }
            }
        }

        void VulkanCommandPoolManager::Shutdown()
        {
            for(auto pool : mGraphicsPools) {
                vkDestroyCommandPool(mDevice.GetHandle(), pool, nullptr);
            }
            for(auto pool : mTransferPools) {
                vkDestroyCommandPool(mDevice.GetHandle(), pool, nullptr);
            }
            for(auto pool : mComputePools) {
                vkDestroyCommandPool(mDevice.GetHandle(), pool, nullptr);
            }
        }

        SPtr<CommandListVk> VulkanCommandPoolManager::AllocateCommandList(const CommandListAllocateInfo& allocateInfo)
        {
            VulkanCommandBuffer commandBuffer = AllocateCommandBuffer(allocateInfo);

            return std::make_shared<CommandListVk>(mDevice, *this, allocateInfo, commandBuffer);
        }

        VulkanCommandBuffer VulkanCommandPoolManager::AllocateCommandBuffer(const CommandListAllocateInfo& allocateInfo)
        {
            VkResult res;

            VkCommandBufferAllocateInfo info;
            info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            info.pNext = nullptr;
            if(allocateInfo.isSub == false) {
                info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            } else {
                info.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
            }
            info.commandBufferCount = 1;

            SPtr<CommandListVk> cmd;
            VkCommandBuffer vkCmdBuf;

            switch(allocateInfo.type) {
                case CommandListType::Graphics: info.commandPool = mGraphicsPools[allocateInfo.poolIndex]; break;
                case CommandListType::Transfer: info.commandPool = mTransferPools[allocateInfo.poolIndex]; break;
                case CommandListType::Compute:  info.commandPool = mComputePools[allocateInfo.poolIndex];  break;
                default:
                    ASSERTION_FAILED("Unknown CommandListType ({})", allocateInfo.type);
            }

            res = vkAllocateCommandBuffers(mDevice.GetHandle(), &info, &vkCmdBuf);
            CHECK_VK(res, "Failed to allocate VkCommandBuffer");
            VULKAN_SET_OBJ_NAME(mDevice.GetHandle(), vkCmdBuf, allocateInfo.debugName);

            VulkanCommandBuffer cmdBuf;
            cmdBuf.handle = vkCmdBuf;
            cmdBuf.type = allocateInfo.type;
            cmdBuf.poolIndex = allocateInfo.poolIndex;

            return cmdBuf;
        }

        void VulkanCommandPoolManager::FreeCommandList(SPtr<CommandListVk>& commandList)
        {
            VulkanCommandBuffer commandBuffer = commandList->GetCommandBuffer();

            FreeCommandBuffer(commandBuffer);
        }

        void VulkanCommandPoolManager::FreeCommandBuffer(VulkanCommandBuffer& commandBuffer)
        {
            CommandListType type = commandBuffer.type;
            Uint32 poolIndex = commandBuffer.poolIndex;
            VkCommandBuffer vkCmdBuf = commandBuffer.handle;

            switch(commandBuffer.type) {
                case CommandListType::Graphics:
                    vkFreeCommandBuffers(mDevice.GetHandle(), mGraphicsPools[poolIndex], 1, &vkCmdBuf);
                    break;
                case CommandListType::Transfer:
                    vkFreeCommandBuffers(mDevice.GetHandle(), mTransferPools[poolIndex], 1, &vkCmdBuf);
                    break;
                case CommandListType::Compute: vkFreeCommandBuffers(mDevice.GetHandle(), mComputePools[poolIndex], 1, &vkCmdBuf);
                    break;
                default:
                    ASSERTION_FAILED("Unknown CommandListType ({})", type);
            }

            commandBuffer.handle = VK_NULL_HANDLE;
        }
    } // namespace rapi
} // namespace cube
