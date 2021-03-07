#pragma once

#include "../VulkanAPIHeader.h"

#include "RenderAPIs/RenderAPI/Interface/Texture.h"

#include "TextureViewVk.h"
#include "../VulkanMemoryAllocator.h"
#include "../VulkanStagingManager.h"

namespace cube
{
    namespace rapi
    {
        class TextureVk : public Texture, public std::enable_shared_from_this<TextureVk>
        {
        public:
            static constexpr Uint32 MaxMipLevels = 16;

        public:
            TextureVk(VulkanDevice& device, const TextureCreateInfo& info);
            virtual ~TextureVk();

            virtual void Map(ResourceMapType type, Uint32 mipmapIndex, void*& pMappedResource) override;
            virtual void Unmap(Uint32 mipmapIndex) override;
            virtual void GenerateMipmaps() override;
            virtual SPtr<Fence> MapAsync(ResourceMapType type, Uint32 mipmapIndex, void*& pMappedResource) override;
            virtual SPtr<Fence> UnmapAsync(Uint32 mipmapIndex) override;
            virtual SPtr<Fence> GenerateMipmapsAsync() override;

            virtual SPtr<TextureView> CreateView(const TextureViewCreateInfo& info) override;
            virtual SPtr<TextureView> CreateDefaultView() override;

            VkImage GetHandle() const { return mImage; }

        private:
            static Uint32 CalculateMipmapLevels(Uint32 width, Uint32 height);

            SPtr<Fence> MapImpl(ResourceMapType type, Uint32 mipIndex, void*& pMappedResource, bool waitUntilFinished);
            SPtr<Fence> UnmapImpl(Uint32 mipIndex, bool waitUntilFinished);
            SPtr<Fence> GenerateMipmapsImpl(bool waitUntilFinished);


            void TransitionLayout(VulkanCommandBuffer& cmdBuf, Int32 mipIndex, VkImageLayout newLayout, bool discardData = false);
            void GetImageBarrierInfos(VkImageLayout layout, VkAccessFlags& outAccessFlags, VkPipelineStageFlags& outPipelineStages);

            VulkanDevice& mDevice;

            VkImage mImage;
            VulkanAllocation mAllocation;

            Array<VkImageLayout, MaxMipLevels> mCurrentLayouts;
            Uint32 mWidth;
            Uint32 mHeight;
            Uint32 mDepth;
            Uint32 mArraySize;
            VkImageAspectFlags mAspectMask;

            Array<VulkanStagingBuffer, MaxMipLevels> mStagingBuffers; // TODO: StagingBuffer들을 가져올 때 해당 리소스 포인터 + 추가 정보를 해싱해서 가져오게 하기

            TextureViewCreateInfo mDefaultViewCreateInfo;
        };
    } // namespace rapi
} // namespace cube
