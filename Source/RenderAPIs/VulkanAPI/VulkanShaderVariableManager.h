#pragma once

#include "VulkanAPIHeader.h"

#include "RenderAPIs/RenderAPI/Utilities/VariableSizeMemoryPage.h"
#include "VulkanMemoryAllocator.h"

namespace cube
{
    namespace rapi
    {
        struct VulkanShaderVariableAllocation
        {
            ShaderVariableType type;
            bool isPerFrame;
            // Uint32 bindIndex; // Assigned in ShaderParameterVk

            VkBuffer buffer;
            Uint64 dynamicOffset;
            Uint64 dynamicUnalignedOffset;

            Uint64 size;
            void* pData;
        };

        constexpr Uint32 UniformBufferMaxSize = 4 * 1024 * 1024;  // 4MiB
        constexpr Uint32 StorageBufferMaxSize = 64 * 1024 * 1024; // 64MiB

        class VulkanShaderVariableHeap
        {
        public:
            VulkanShaderVariableHeap(VulkanDevice& device) : mDevice(device)
            {}
            ~VulkanShaderVariableHeap() = default;

            void Initialize(ShaderVariableType type, Uint32 heapIndex);
            void Shutdown();

            VkBuffer GetHeapBuffer() const { return mBuffer; }

            VulkanShaderVariableAllocation Allocate(Uint64 size);
            void Free(VulkanShaderVariableAllocation& allocation);

        private:
            VulkanDevice& mDevice;

            Uint32 mHeapIndex;
            ShaderVariableType mHeapType;

            VkBuffer mBuffer;
            VulkanAllocation mBufferMemoryAllocation;
            Uint64 mMinAlignment;

            VariableSizeMemoryPage mBufferPage;
        };

        class VulkanShaderVariableManager
        {
        public:
            VulkanShaderVariableManager(VulkanDevice& device);
            ~VulkanShaderVariableManager() = default;

            VulkanShaderVariableManager(const VulkanShaderVariableManager& other) = delete;
            VulkanShaderVariableManager& operator=(const VulkanShaderVariableManager& rhs) = delete;

            void Initialize();
            void Shutdown();

            VulkanShaderVariableAllocation Allocate(ShaderVariableType type, Uint64 size); // TODO: 공간 없을 때 Heap 새로 생성해서 반환하는 기능 추가
            void Free(VulkanShaderVariableAllocation& allocation);

            VkDescriptorSet AllocateDescriptorSet(VkDescriptorSetLayout layout, const char* layoutDebugName);
            void FreeDescriptorSet(VkDescriptorSet descSet);

            VkBuffer GetUniformDynamicBuffer();
            VkBuffer GetStorageDynamicBuffer();

        private:
            VulkanDevice& mDevice;

            VulkanShaderVariableHeap mUniformHeap;
            VulkanShaderVariableHeap mStorageHeap;

            VkDescriptorPool mDescriptorPool;
        };
    } // namespace rapi
} // namespace cube
