#include "VulkanShaderVariableManager.h"

#include "Core/Allocator/FrameAllocator.h"
#include "VulkanDevice.h"
#include "VulkanDebug.h"
#include "Core/Assertion.h"

namespace cube
{
    namespace rapi
    {
        void VulkanShaderVariableHeap::Initialize(ShaderVariableType type, Uint32 heapIndex)
        {
            mHeapIndex = heapIndex;
            mHeapType = type;

            VkResult res;

            auto& limits = mDevice.GetProperties().limits;

            VkBufferCreateInfo bufInfo;
            bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufInfo.pNext = nullptr;
            bufInfo.flags = 0;
            bufInfo.queueFamilyIndexCount = 0;
            bufInfo.pQueueFamilyIndices = nullptr;
            bufInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            Uint32 bufSize = 0;
            const char* bufDebugName = "";
            switch(type) {
                case ShaderVariableType::Constant:
                    bufSize = (UniformBufferMaxSize > limits.maxUniformBufferRange) ? limits.maxUniformBufferRange : UniformBufferMaxSize;
                    bufInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
                    bufDebugName = "Uniform buffer in VulkanShaderVariableHeap";
                    mMinAlignment = limits.minUniformBufferOffsetAlignment;
                    break;

                case ShaderVariableType::Storage:
                    bufSize = (StorageBufferMaxSize > limits.maxStorageBufferRange) ? limits.maxStorageBufferRange : StorageBufferMaxSize;
                    bufInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
                    bufDebugName = "Storage buffer in VulkanShaderVariableHeap";
                    mMinAlignment = limits.minStorageBufferOffsetAlignment;
                    break;

                default:
                    ASSERTION_FAILED("Unknown VulkanShaderVariableType ({0})", (Uint32)type);
            }
            bufInfo.size = bufSize;
            res = vkCreateBuffer(mDevice.GetHandle(), &bufInfo, nullptr, &mBuffer);
            CHECK_VK(res, "Failed to create VkBuffer.");
            VULKAN_SET_OBJ_NAME(mDevice.GetHandle(), mBuffer, bufDebugName);

            mBufferMemoryAllocation = mDevice.GetAllocator().Allocate(ResourceUsage::Staging, bufInfo, &mBuffer);

            mBufferPage = VariableSizeMemoryPage(mBufferMemoryAllocation.size, 0);
        }

        void VulkanShaderVariableHeap::Shutdown()
        {
            mDevice.GetAllocator().Free(mBufferMemoryAllocation);
            vkDestroyBuffer(mDevice.GetHandle(), mBuffer, nullptr);
        }

        VulkanShaderVariableAllocation VulkanShaderVariableHeap::Allocate(Uint64 size)
        {
            VariableSizeMemoryPage::Allocation alloc = mBufferPage.Allocate(size, mMinAlignment);

            VulkanShaderVariableAllocation allocation;
            allocation.type = mHeapType;
            allocation.isPerFrame = false;
            allocation.buffer = mBuffer;
            allocation.pData = (char*)mBufferMemoryAllocation.pMappedPtr + alloc.offset;
            allocation.size = size;
            allocation.dynamicOffset = alloc.offset;
            allocation.dynamicUnalignedOffset = alloc.unalignedOffset;

            return allocation;
        }

        void VulkanShaderVariableHeap::Free(VulkanShaderVariableAllocation& allocation)
        {
            VariableSizeMemoryPage::Allocation alloc{ allocation.dynamicOffset, allocation.dynamicUnalignedOffset, allocation.size };
            mBufferPage.Free(alloc);

            allocation.size = 0;
            allocation.pData = nullptr;
        }

        VulkanShaderVariableManager::VulkanShaderVariableManager(VulkanDevice& device) :
            mDevice(device),
            mUniformHeap(device),
            mStorageHeap(device)
        {

        }

        void VulkanShaderVariableManager::Initialize()
        {
            mUniformHeap.Initialize(ShaderVariableType::Constant, 0);
            mStorageHeap.Initialize(ShaderVariableType::Storage, 1);

            VkResult res;

            auto& limits = mDevice.GetProperties().limits;

            VkDescriptorPoolCreateInfo descInfo;
            descInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            descInfo.pNext = nullptr;
            descInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
            descInfo.maxSets = 4096;
            FrameVector<VkDescriptorPoolSize> poolSize
            {
                { VK_DESCRIPTOR_TYPE_SAMPLER,                limits.maxDescriptorSetSamplers },
                { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, limits.maxDescriptorSetSampledImages },
                { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,          limits.maxDescriptorSetSampledImages },
                { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,          limits.maxDescriptorSetStorageImages },
                { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,   limits.maxDescriptorSetUniformBuffers },
                { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,   limits.maxDescriptorSetStorageBuffers },
                { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         limits.maxDescriptorSetUniformBuffers },
                { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         limits.maxDescriptorSetStorageBuffers },
                { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, limits.maxDescriptorSetUniformBuffersDynamic },
                { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, limits.maxDescriptorSetStorageBuffersDynamic },
                { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,       limits.maxDescriptorSetInputAttachments }
            };
            descInfo.poolSizeCount = SCast(Uint32)(poolSize.size());
            descInfo.pPoolSizes = poolSize.data();

            res = vkCreateDescriptorPool(mDevice.GetHandle(), &descInfo, nullptr, &mDescriptorPool);
            CHECK_VK(res, "Failed to create VkDescriptorPool.");
            VULKAN_SET_OBJ_NAME(mDevice.GetHandle(), mDescriptorPool, "DescriptorPool in VulkanShaderVariableManager");
        }

        void VulkanShaderVariableManager::Shutdown()
        {
            vkDestroyDescriptorPool(mDevice.GetHandle(), mDescriptorPool, nullptr);

            mStorageHeap.Shutdown();
            mUniformHeap.Shutdown();
        }

        VulkanShaderVariableAllocation VulkanShaderVariableManager::Allocate(ShaderVariableType type, Uint64 size)
        {
            switch(type) {
                case ShaderVariableType::Constant:
                    return mUniformHeap.Allocate(size);
                case ShaderVariableType::Storage:
                    return mStorageHeap.Allocate(size);
                default:
                    ASSERTION_FAILED("Unknown VulkanShaderVariableType ({0})", (Uint32)type);
            }
            return {};
        }

        void VulkanShaderVariableManager::Free(VulkanShaderVariableAllocation& allocation)
        {
            switch(allocation.type) {
                case ShaderVariableType::Constant:
                    mUniformHeap.Free(allocation);
                    break;
                case ShaderVariableType::Storage:
                    mStorageHeap.Free(allocation);
                    break;
            }
        }

        VkDescriptorSet VulkanShaderVariableManager::AllocateDescriptorSet(VkDescriptorSetLayout layout, const char* layoutDebugName)
        {
            VkResult res;

            VkDescriptorSetAllocateInfo info;
            info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            info.pNext = nullptr;
            info.descriptorPool = mDescriptorPool;
            info.descriptorSetCount = 1;
            info.pSetLayouts = &layout;

            VkDescriptorSet set;
            res = vkAllocateDescriptorSets(mDevice.GetHandle(), &info, &set);
            CHECK_VK(res, "Failed to allocate descriptor set.");

            VULKAN_SET_OBJ_NAME(mDevice.GetHandle(), set, FrameFormat("DescriptorSet in {}", layoutDebugName).c_str());

            return set;
        }

        void VulkanShaderVariableManager::FreeDescriptorSet(VkDescriptorSet descSet)
        {
            VkResult res;

            res = vkFreeDescriptorSets(mDevice.GetHandle(), mDescriptorPool, 1, &descSet);
            CHECK_VK(res, "Failed to free descriptor set.");
        }

        VkBuffer VulkanShaderVariableManager::GetUniformDynamicBuffer()
        {
            return mUniformHeap.GetHeapBuffer();
        }

        VkBuffer VulkanShaderVariableManager::GetStorageDynamicBuffer()
        {
            return mStorageHeap.GetHeapBuffer();
        }
    } // namespace rapi
} // namespace cube
