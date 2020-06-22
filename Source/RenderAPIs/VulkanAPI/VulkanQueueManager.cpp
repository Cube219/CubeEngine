#include "VulkanQueueManager.h"

#include "Core/Assertion.h"
#include "Core/Allocator/FrameAllocator.h"

#include "VulkanDevice.h"

namespace cube
{
    namespace rapi
    {
        void VulkanQueueManager::Initialize(VkPhysicalDevice gpu)
        {
            Uint32 queueFamilyNum = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueFamilyNum, nullptr);
            CHECK(queueFamilyNum > 0, "Cannot find physical device queue family.");

            FrameVector<VkQueueFamilyProperties> queueFamilyProps(queueFamilyNum);
            vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueFamilyNum, queueFamilyProps.data());

            bool res;
            res = InitGraphicsQueue(queueFamilyProps.data(), queueFamilyNum);
            CHECK(res, "Cannot find graphics queue family.");

            res = InitComputeQueue(queueFamilyProps.data(), queueFamilyNum);
            CHECK(res, "Cannot find compute queue family.");

            res = InitTransferQueue(queueFamilyProps.data(), queueFamilyNum);
            CHECK(res, "Cannot find transfer queue family.");
        }

        void VulkanQueueManager::Shutdown()
        {
        }

        bool VulkanQueueManager::InitGraphicsQueue(VkQueueFamilyProperties* pProps, Uint64 propNum)
        {
            Uint32 familyIndex = FindQueueFamilyIndex(pProps, propNum, VK_QUEUE_GRAPHICS_BIT, 0);
            if(familyIndex == Uint32InvalidValue) {
                return false;
            }
            // Only use the first queue in queue family
            vkGetDeviceQueue(mDevice.GetHandle(), familyIndex, 0, &mGraphicsQueue.queue);
            mGraphicsQueue.familyIndex = familyIndex;
            mGraphicsQueue.queueIndex = 0;

            return true;
        }

        bool VulkanQueueManager::InitComputeQueue(VkQueueFamilyProperties* pProps, Uint64 propNum)
        {
            // Find queue family with compute bit and without graphics bit
            Uint32 familyIndex = FindQueueFamilyIndex(pProps, propNum, VK_QUEUE_COMPUTE_BIT, VK_QUEUE_GRAPHICS_BIT);
            if(familyIndex != Uint32InvalidValue) {
                Uint32 queueCount = pProps[familyIndex].queueCount;
                mComputeQueues.resize(queueCount);
                for(Uint32 i = 0; i < queueCount; i++) {
                    vkGetDeviceQueue(mDevice.GetHandle(), familyIndex, i, &mComputeQueues[i].queue);
                    mComputeQueues[i].familyIndex = familyIndex;
                    mComputeQueues[i].queueIndex = i;
                }
                return true;
            }

            // If not found, find with compute bit
            familyIndex = FindQueueFamilyIndex(pProps, propNum, VK_QUEUE_COMPUTE_BIT, 0);
            if(familyIndex == Uint32InvalidValue)
                return false;

            Uint32 queueCount = pProps[familyIndex].queueCount;
            mComputeQueues.resize(queueCount);

            Uint32 queueIndex = 0;
            for(Uint32 i = 0; i < queueCount; i++) {
                // If the queue family is graphics, skip its queue
                // because it is used for graphics only
                if(mGraphicsQueue.familyIndex == familyIndex && mGraphicsQueue.queueIndex == i) {
                    mComputeQueues.resize(queueCount - 1);
                    continue;
                }

                vkGetDeviceQueue(mDevice.GetHandle(), familyIndex, i, &mComputeQueues[queueIndex].queue);
                mComputeQueues[queueIndex].familyIndex = familyIndex;
                mComputeQueues[queueIndex].queueIndex = i;
                queueIndex++;
            }

            // If the queue family index is graphics but has only one queue,
            // jist add its queue
            if(queueIndex == 0) {
                mComputeQueues.resize(1);
                vkGetDeviceQueue(mDevice.GetHandle(), familyIndex, 0, &mComputeQueues[0].queue);
                mComputeQueues[0].familyIndex = familyIndex;
                mComputeQueues[0].queueIndex = 0;
            }

            return true;
        }

        bool VulkanQueueManager::InitTransferQueue(VkQueueFamilyProperties* pProps, Uint64 propNum)
        {
            // Find queue family with transfer bit only
            Uint32 familyIndex = FindQueueFamilyIndex(pProps, propNum, VK_QUEUE_TRANSFER_BIT, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT);
            if(familyIndex != Uint32InvalidValue) {
                Uint32 queueCount = pProps[familyIndex].queueCount;
                mTransferQueues.resize(queueCount);
                for(Uint32 i = 0; i < queueCount; i++) {
                    vkGetDeviceQueue(mDevice.GetHandle(), familyIndex, i, &mTransferQueues[i].queue);
                    mTransferQueues[i].familyIndex = familyIndex;
                    mTransferQueues[i].queueIndex = i;
                }
                return true;
            }

            // If not found, find with transfer bit
            familyIndex = FindQueueFamilyIndex(pProps, propNum, VK_QUEUE_TRANSFER_BIT, 0);
            if(familyIndex == Uint32InvalidValue)
                return false;

            Uint32 queueCount = pProps[familyIndex].queueCount;
            mTransferQueues.resize(queueCount);

            Uint32 queueIndex = 0;
            for(Uint32 i = 0; i < queueCount; i++) {
                // If the queue family is graphics, skip its queue
                // because it is used for graphics only
                if(mGraphicsQueue.familyIndex == familyIndex && mGraphicsQueue.queueIndex == i) {
                    mTransferQueues.resize(queueCount - 1);
                    continue;
                }

                vkGetDeviceQueue(mDevice.GetHandle(), familyIndex, i, &mTransferQueues[queueIndex].queue);
                mTransferQueues[queueIndex].familyIndex = familyIndex;
                mTransferQueues[queueIndex].queueIndex = i;
                queueIndex++;
            }

            // If the queue family index is graphics but has only one queue,
            // jist add its queue
            if(queueIndex == 0) {
                mTransferQueues.resize(1);
                vkGetDeviceQueue(mDevice.GetHandle(), familyIndex, 0, &mTransferQueues[0].queue);
                mTransferQueues[0].familyIndex = familyIndex;
                mTransferQueues[0].queueIndex = 0;
            }

            return true;
        }

        Uint32 VulkanQueueManager::FindQueueFamilyIndex(VkQueueFamilyProperties* pProps, Uint64 propNum, VkQueueFlags flags, VkQueueFlags prohibitFlags)
        {
            for(Uint32 i = 0; i < propNum; i++) {
                if((pProps[i].queueFlags & flags) != 0
                    && (pProps[i].queueFlags & prohibitFlags) == 0) {
                    return i;
                }
            }

            return Uint32InvalidValue;
        }
    } // namespace rapi
} // namespace cube
