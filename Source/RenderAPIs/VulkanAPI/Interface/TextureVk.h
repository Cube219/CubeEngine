#pragma once

#include "../VulkanAPIHeader.h"

#include "RenderAPIs/RenderAPI/Interface/Texture.h"

#include "../VulkanMemoryAllocator.h"
#include "../VulkanStagingManager.h"

namespace cube
{
    namespace rapi
    {
        class TextureVk
        {
        public:
            static constexpr Uint32 MaxMipLevels = 16;

        public:
            TextureVk(VulkanDevice& device, ResourceUsage usage, VkImageViewType type, FrameVector<TextureData> data,
                TextureFormat format, Uint32 width, Uint32 height, Uint32 depth, Uint32 arraySize,
                TextureBindTypeFlags bindTypeFlags, bool generateMipmaps, Uint32 samplesNum, const char* debugName);
            virtual ~TextureVk();

            SPtr<Fence> MapImpl(ResourceMapType type, Uint32 mipIndex, void*& pMappedResource, bool waitUntilFinished);
            SPtr<Fence> UnmapImpl(Uint32 mipIndex, bool waitUntilFinished);

            Uint32 GetMipLevelsInTextureVk() const { return mMipLevelsInVk; }

        private:
            static Uint32 CalculateMipmapLevels(Uint32 width, Uint32 height);
            void GenerateMipmaps(VulkanCommandBuffer& cmdBuf);

            void TransitionLayout(VulkanCommandBuffer& cmdBuf, Int32 mipIndex, VkImageLayout newLayout, bool discardData = false);
            void GetImageBarrierInfos(VkImageLayout layout, VkAccessFlags& outAccessFlags, VkPipelineStageFlags& outPipelineStages);

            VulkanDevice& mDevice;

            VkImage mImage;
            VulkanAllocation mAllocation;

            Array<VkImageLayout, MaxMipLevels> mCurrentLayouts;
            Uint32 mMipLevelsInVk;
            Uint32 mWidth;
            Uint32 mHeight;
            Uint32 mDepth;
            ResourceUsage mUsage;
            VkImageAspectFlags mAspectMask;
            VkFormat mFormat;
            Array<VulkanStagingBuffer, MaxMipLevels> mStagingBuffers; // TODO: StagingBuffer들을 가져올 때 해당 리소스 포인터 + 추가 정보를 해싱해서 가져오게 하기
            const char* mDebugName;
        };

        class Texture2DVk : public Texture2D, public TextureVk
        {
        public:
            Texture2DVk(VulkanDevice& device, const Texture2DCreateInfo& info);
            virtual ~Texture2DVk() {}

            virtual void Map(ResourceMapType type, Uint32 mipIndex, void*& pMappedResource) override
            {
                MapImpl(type, mipIndex, pMappedResource, true);
            }
            virtual void Unmap(Uint32 mipIndex) override
            {
                UnmapImpl(mipIndex, true);
            }

            virtual SPtr<Fence> MapAsync(ResourceMapType type, Uint32 mipmapIndex, void*& pMappedResource) override
            {
                return MapImpl(type, mipmapIndex, pMappedResource, false);
            }
            virtual SPtr<Fence> UnmapAsync(Uint32 mipmapIndex) override
            {
                return UnmapImpl(mipmapIndex, false);
            }
        };

        class Texture2DArrayVk : public Texture2DArray, public TextureVk
        {
        public:
            Texture2DArrayVk(VulkanDevice& device, const Texture2DArrayCreateInfo& info);
            virtual ~Texture2DArrayVk() {}

            virtual void Map(ResourceMapType type, Uint32 mipIndex, void*& pMappedResource) override
            {
                MapImpl(type, mipIndex, pMappedResource, true);
            }
            virtual void Unmap(Uint32 mipIndex) override
            {
                UnmapImpl(mipIndex, true);
            }

            virtual SPtr<Fence> MapAsync(ResourceMapType type, Uint32 mipmapIndex, void*& pMappedResource) override
            {
                return MapImpl(type, mipmapIndex, pMappedResource, false);
            }
            virtual SPtr<Fence> UnmapAsync(Uint32 mipmapIndex) override
            {
                return UnmapImpl(mipmapIndex, false);
            }
        };

        class Texture3DVk : public Texture3D, public TextureVk
        {
        public:
            Texture3DVk(VulkanDevice& device, const Texture3DCreateInfo& info);
            virtual ~Texture3DVk() {}

            virtual void Map(ResourceMapType type, Uint32 mipIndex, void*& pMappedResource) override
            {
                MapImpl(type, mipIndex, pMappedResource, true);
            }
            virtual void Unmap(Uint32 mipIndex) override
            {
                UnmapImpl(mipIndex, true);
            }

            virtual SPtr<Fence> MapAsync(ResourceMapType type, Uint32 mipmapIndex, void*& pMappedResource) override
            {
                return MapImpl(type, mipmapIndex, pMappedResource, false);
            }
            virtual SPtr<Fence> UnmapAsync(Uint32 mipmapIndex) override
            {
                return UnmapImpl(mipmapIndex, false);
            }
        };

        class TextureCubeVk : public TextureCube, public TextureVk
        {
        public:
            TextureCubeVk(VulkanDevice& device, const TextureCubeCreateInfo& info);
            virtual ~TextureCubeVk() {}

            virtual void Map(ResourceMapType type, Uint32 mipIndex, void*& pMappedResource) override
            {
                MapImpl(type, mipIndex, pMappedResource, true);
            }
            virtual void Unmap(Uint32 mipIndex) override
            {
                UnmapImpl(mipIndex, true);
            }

            virtual SPtr<Fence> MapAsync(ResourceMapType type, Uint32 mipmapIndex, void*& pMappedResource) override
            {
                return MapImpl(type, mipmapIndex, pMappedResource, false);
            }
            virtual SPtr<Fence> UnmapAsync(Uint32 mipmapIndex) override
            {
                return UnmapImpl(mipmapIndex, false);
            }
        };
    } // namespace rapi
} // namespace cube
