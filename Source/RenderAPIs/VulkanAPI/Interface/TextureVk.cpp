#include "TextureVk.h"

#include "FenceVk.h"
#include "../VulkanDebug.h"
#include "../VulkanDevice.h"
#include "../VulkanTypeConversion.h"
#include "../VulkanUtility.h"

#include "Core/Assertion.h"
#include "Core/LogWriter.h"
#include "Utility/Math.h"

namespace cube
{
    namespace rapi
    {
        TextureVk::TextureVk(VulkanDevice& device, const TextureCreateInfo& info) :
            Texture(info.usage, info.debugName),
            mDevice(device),
            mWidth(info.width), mHeight(info.height), mDepth(info.depth),
            mArraySize(info.arraySize),
            mAspectMask(0)
        {
            mMipLevels = CalculateMipmapLevels(info.width, info.height);
            if(info.generateMipmaps == false) {
                mMipLevels = Math::Min(mMipLevels, SCast(Uint32)(info.data.size()));
                mMipLevels = Math::Min(mMipLevels, MaxMipLevels);
            }

            VkResult res;

            VkImageCreateInfo imageCreateInfo;
            imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageCreateInfo.pNext = nullptr;
            imageCreateInfo.flags = 0;
            switch(info.dimension)
            {
                case TextureDimension::Texture1D:
                case TextureDimension::Texture1DArray:
                    imageCreateInfo.imageType = VK_IMAGE_TYPE_1D;
                    break;
                case TextureDimension::TextureCube:
                case TextureDimension::TextureCubeArray:
                    imageCreateInfo.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
                    // Fallthrough
                case TextureDimension::Texture2D:
                case TextureDimension::Texture2DArray:
                    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
                    break;
                case TextureDimension::Texture3D:
                    imageCreateInfo.imageType = VK_IMAGE_TYPE_3D;
                    break;

                default:
                    ASSERTION_FAILED("Invalid texture dimension {}.", info.dimension);
                    break;
            }
            imageCreateInfo.format = TextureFormatToVkFormat(mFormat);
            CHECK(imageCreateInfo.format != VK_FORMAT_UNDEFINED, "Texture format ({}) is not defined.", (int)info.format);
            imageCreateInfo.extent = { info.width, info.height, info.depth };
            imageCreateInfo.mipLevels = mMipLevels;
            imageCreateInfo.arrayLayers = info.arraySize;
            switch(info.samplesNum)
            {
                case 1:  imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;  break;
                case 2:  imageCreateInfo.samples = VK_SAMPLE_COUNT_2_BIT;  break;
                case 4:  imageCreateInfo.samples = VK_SAMPLE_COUNT_4_BIT;  break;
                case 8:  imageCreateInfo.samples = VK_SAMPLE_COUNT_8_BIT;  break;
                case 16: imageCreateInfo.samples = VK_SAMPLE_COUNT_16_BIT; break;
                case 32: imageCreateInfo.samples = VK_SAMPLE_COUNT_32_BIT; break;
                case 64: imageCreateInfo.samples = VK_SAMPLE_COUNT_64_BIT; break;
                default:
                    CUBE_LOG(LogType::Warning, "Unsupported sampler count ({}). Use sampler count 1 instead.", info.samplesNum);
                    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
                    break;
            }
            imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            mCurrentLayouts.fill(VK_IMAGE_LAYOUT_UNDEFINED);
            imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
            if(info.bindTypeFlags.IsSet(TextureBindTypeFlag::RenderTarget)) {
                imageCreateInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
                mAspectMask |= VK_IMAGE_ASPECT_COLOR_BIT;
            }
            if(info.bindTypeFlags.IsSet(TextureBindTypeFlag::ShaderResource)) {
                imageCreateInfo.usage |= VK_IMAGE_USAGE_STORAGE_BIT;
                mAspectMask |= VK_IMAGE_ASPECT_COLOR_BIT;
            }
            if(info.bindTypeFlags.IsSet(TextureBindTypeFlag::DepthStencil)) {
                imageCreateInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
                mAspectMask |= (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
            }
            imageCreateInfo.queueFamilyIndexCount = 0;
            imageCreateInfo.pQueueFamilyIndices = nullptr;
            imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;

            res = vkCreateImage(device.GetHandle(), &imageCreateInfo, nullptr, &mImage);
            CHECK_VK(res, "Failed to craete VkImage.");
            VULKAN_SET_OBJ_NAME(device.GetHandle(), mImage, info.debugName);

            // Allocate memory
            mAllocation = device.GetAllocator().Allocate(info.usage, imageCreateInfo, &mImage);

            // Initialize data if it is existed
            if(info.data.size() > 0) {
                // Change the usage temporarily to enable map
                if(info.usage == ResourceUsage::Immutable) mUsage = ResourceUsage::Default;

                Uint32 numToCreate = mMipLevels;
                if(info.generateMipmaps) {
                    numToCreate = 1;
                }

                for(Uint32 i = 0; i < numToCreate; i++) {
                    void* p;
                    MapImpl(ResourceMapType::Write, i, p, true);
                    memcpy(p, info.data[i].pData, info.data[i].size);
                    UnmapImpl(i, true);
                }

                if(info.generateMipmaps) {
                    GenerateMipmapsImpl(true);
                }

                mUsage = info.usage;
            }

            // Create DefaultViewCreateInfo
            TextureViewType viewType = TextureViewType::UnorderedAccessView;
            if(info.bindTypeFlags.IsSet(TextureBindTypeFlag::RenderTarget)) {
                viewType = TextureViewType::RenderTargetView;
            } else if(info.bindTypeFlags.IsSet(TextureBindTypeFlag::DepthStencil)) {
                viewType = TextureViewType::DepthStencilView;
            } else if(info.bindTypeFlags.IsSet(TextureBindTypeFlag::ShaderResource)) {
                viewType = TextureViewType::ShaderResourceView;
            }
            mDefaultViewCreateInfo.type = viewType;
            TextureViewDimension viewDimension;
            switch(info.dimension) {
                case TextureDimension::Texture1D:        viewDimension = TextureViewDimension::TextureView1D;        break;
                case TextureDimension::Texture1DArray:   viewDimension = TextureViewDimension::TextureView1DArray;   break;
                case TextureDimension::Texture2D:        viewDimension = TextureViewDimension::TextureView2D;        break;
                case TextureDimension::Texture2DArray:   viewDimension = TextureViewDimension::TextureView2DArray;   break;
                case TextureDimension::Texture3D:        viewDimension = TextureViewDimension::TextureView3D;        break;
                case TextureDimension::TextureCube:      viewDimension = TextureViewDimension::TextureViewCube;      break;
                case TextureDimension::TextureCubeArray: viewDimension = TextureViewDimension::TextureViewCubeArray; break;
            }
            mDefaultViewCreateInfo.dimension = viewDimension;
            mDefaultViewCreateInfo.format = info.format;
            mDefaultViewCreateInfo.mipmapStartIndex = 0;
            mDefaultViewCreateInfo.numMipmaps = 0;
            mDefaultViewCreateInfo.arrayStartIndex = 0;
            mDefaultViewCreateInfo.numArrays = 0;
            mDefaultViewCreateInfo.debugName = "Default Texture View";
        }

        TextureVk::~TextureVk()
        {
            mDevice.GetAllocator().Free(mAllocation);
            for(auto& stagingBuf : mStagingBuffers) {
                if(stagingBuf.IsValid()) {
                    mDevice.GetStagingManager().ReleaseBuffer(std::move(stagingBuf));
                }
            }
            vkDestroyImage(mDevice.GetHandle(), mImage, nullptr);
        }

        void TextureVk::Map(ResourceMapType type, Uint32 mipmapIndex, void*& pMappedResource)
        {
            MapImpl(type, mipmapIndex, pMappedResource, true);
        }

        void TextureVk::Unmap(Uint32 mipmapIndex)
        {
            UnmapImpl(mipmapIndex, true);
        }

        void TextureVk::GenerateMipmaps()
        {
            GenerateMipmapsImpl(true);
        }

        SPtr<Fence> TextureVk::MapAsync(ResourceMapType type, Uint32 mipmapIndex, void*& pMappedResource)
        {
            return MapImpl(type, mipmapIndex, pMappedResource, false);
        }

        SPtr<Fence> TextureVk::UnmapAsync(Uint32 mipmapIndex)
        {
            return UnmapImpl(mipmapIndex, false);
        }

        SPtr<Fence> TextureVk::GenerateMipmapsAsync()
        {
            return GenerateMipmapsImpl(false);
        }

        SPtr<TextureView> TextureVk::CreateView(const TextureViewCreateInfo& info)
        {
            SPtr<TextureVk> mine = shared_from_this();
            return std::make_shared<TextureViewVk>(mDevice, mine , info);
        }

        SPtr<TextureView> TextureVk::CreateDefaultView()
        {
            SPtr<TextureView> view = CreateView(mDefaultViewCreateInfo);
            return view;
        }

        SPtr<Fence> TextureVk::MapImpl(ResourceMapType type, Uint32 mipIndex, void*& pMappedResource, bool waitUntilFinished)
        {
            CHECK(mUsage != ResourceUsage::Immutable, "Cannot map immutable resource.");
            if(mUsage == ResourceUsage::Staging) {
                CHECK(type == ResourceMapType::Read, "Cannot map staging resource in write mode.");
            }

            if(mUsage == ResourceUsage::Dynamic) {
                CHECK(mipIndex == 0, "Cannot mapping more that 0 index in dynamic texture.")

                // Dynamic resource is always mapped
                pMappedResource = mAllocation.pMappedPtr;
                if(waitUntilFinished == true) {
                    return nullptr;
                } else {
                    return mDevice.GetFencePool().CreateNullFence();
                }
            }

            VulkanStagingBuffer::Type stagingBufType;
            switch(type)
            {
                case ResourceMapType::Read:      stagingBufType = VulkanStagingBuffer::Type::Read; break;
                case ResourceMapType::Write:     stagingBufType = VulkanStagingBuffer::Type::Write; break;
                case ResourceMapType::ReadWrite: // Fallthrough
                default:
                    stagingBufType = VulkanStagingBuffer::Type::ReadWrite;
                    break;
            }
            Uint32 mipWidth = Math::Max(mWidth >> mipIndex, 1u);
            Uint32 mipHeight = Math::Max(mHeight >> mipIndex, 1u);
            Uint32 mipDepth = Math::Max(mDepth >> mipIndex, 1u);
            Uint64 bufSize = mipWidth * mipHeight * mipDepth * mArraySize * 4;
            mStagingBuffers[mipIndex] = mDevice.GetStagingManager().GetBuffer(bufSize, stagingBufType, mDebugName);
            pMappedResource = mStagingBuffers[mipIndex].GetMappedPtr();

            if(type != ResourceMapType::Write) {
                mStagingBuffers[mipIndex].InitCommandBuffer();

                TransitionLayout(mStagingBuffers[mipIndex].GetCommandBuffer(), mipIndex, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

                VkBufferImageCopy region;
                region.bufferOffset = 0;
                region.bufferRowLength = 0;
                region.bufferImageHeight = 0;
                region.imageSubresource.aspectMask = mAspectMask;
                region.imageSubresource.mipLevel = mipIndex;
                region.imageSubresource.baseArrayLayer = 0;
                region.imageSubresource.layerCount = VK_REMAINING_ARRAY_LAYERS;
                region.imageOffset = { 0, 0, 0 };
                region.imageExtent = { mipWidth, mipHeight, mipDepth };
                mStagingBuffers[mipIndex].CopyImage(mImage, region);

                TransitionLayout(mStagingBuffers[mipIndex].GetCommandBuffer(), mipIndex, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

                auto fence = mStagingBuffers[mipIndex].SubmitCommandBuffer();
                if(waitUntilFinished == true) {
                    fence->Wait(10.0f);
                    return nullptr;
                } else {
                    return fence;
                }
            }

            if(waitUntilFinished == true) {
                return nullptr;
            } else {
                return mDevice.GetFencePool().CreateNullFence();
            }
        }

        SPtr<Fence> TextureVk::UnmapImpl(Uint32 mipIndex, bool waitUntilFinished)
        {
            CHECK(mUsage != ResourceUsage::Immutable, "Cannot unmap immutable resource.");

            if(mUsage == ResourceUsage::Dynamic) {
                // Dynamic resource is always mapped
                if(waitUntilFinished == true) {
                    return nullptr;
                } else {
                    return mDevice.GetFencePool().CreateNullFence();
                }
            }

            Uint32 mipWidth = Math::Max(mWidth >> mipIndex, 1u);
            Uint32 mipHeight = Math::Max(mHeight >> mipIndex, 1u);
            Uint32 mipDepth = Math::Max(mDepth >> mipIndex, 1u);

            if(mStagingBuffers[mipIndex].IsValid() && mStagingBuffers[mipIndex].GetType() != VulkanStagingBuffer::Type::Read) {
                mStagingBuffers[mipIndex].InitCommandBuffer();

                TransitionLayout(mStagingBuffers[mipIndex].GetCommandBuffer(), mipIndex, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, true);

                VkBufferImageCopy region;
                region.bufferOffset = 0;
                region.bufferRowLength = 0;
                region.bufferImageHeight = 0;
                region.imageSubresource.aspectMask = mAspectMask;
                region.imageSubresource.mipLevel = mipIndex;
                region.imageSubresource.baseArrayLayer = 0;
                region.imageSubresource.layerCount = VK_REMAINING_ARRAY_LAYERS;
                region.imageOffset = { 0, 0, 0 };
                region.imageExtent = { mipWidth, mipHeight, mipDepth };
                mStagingBuffers[mipIndex].FlushImage(mImage, region);

                TransitionLayout(mStagingBuffers[mipIndex].GetCommandBuffer(), mipIndex, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

                auto fence = mStagingBuffers[mipIndex].SubmitCommandBuffer();
                mDevice.GetStagingManager().ReleaseBuffer(std::move(mStagingBuffers[mipIndex]));
                if(waitUntilFinished == true) {
                    fence->Wait(10.0f);
                    return nullptr;
                } else {
                    return fence;
                }
            }

            if(mStagingBuffers[mipIndex].IsValid() == true) {
                mDevice.GetStagingManager().ReleaseBuffer(std::move(mStagingBuffers[mipIndex]));
            }
            
            if(waitUntilFinished == true) {
                return nullptr;
            } else {
                return mDevice.GetFencePool().CreateNullFence();
            }
        }

        SPtr<Fence> TextureVk::GenerateMipmapsImpl(bool waitUntilFinished)
        {
            VkPhysicalDevice gpu = mDevice.GetGPU();
            VkFormatProperties formatProps;
            VkFormat vkFormat = TextureFormatToVkFormat(mFormat);
            vkGetPhysicalDeviceFormatProperties(gpu, vkFormat, &formatProps);
            if((formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT) == 0) {
                CUBE_LOG(LogType::Warning, "Cannot generate mipmaps in format({}) in this gpu. Skip it.", (int)vkFormat);
                return mDevice.GetFencePool().CreateNullFence();
            }

            CommandListAllocateInfo cmdInfo;
            cmdInfo.type = CommandListType::Transfer;

            VulkanCommandBuffer cmdBuf = mDevice.GetCommandPoolManager().AllocateCommandBuffer(cmdInfo);

            Int32 mipWidth = mWidth;
            Int32 mipHeight = mHeight;
            Int32 mipDepth = mDepth;

            TransitionLayout(cmdBuf, 0, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

            VkImageBlit blit;
            blit.srcSubresource.aspectMask = mAspectMask;
            blit.srcSubresource.baseArrayLayer = 0;
            blit.srcSubresource.layerCount = VK_REMAINING_ARRAY_LAYERS;
            blit.srcOffsets[0] = { 0, 0, 0 };

            blit.dstSubresource.aspectMask = mAspectMask;
            blit.dstSubresource.baseArrayLayer = 0;
            blit.dstSubresource.layerCount = VK_REMAINING_ARRAY_LAYERS;
            blit.dstOffsets[0] = { 0, 0, 0 };

            for(Uint32 i = 1; i < mMipLevels; i++) {
                TransitionLayout(cmdBuf, i, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

                blit.srcSubresource.mipLevel = i - 1;
                blit.srcOffsets[1] = { mipWidth, mipHeight, mipDepth };

                Int32 newMipWidth = Math::Max(mipWidth >> 2, 1);
                Int32 newMipHeight = Math::Max(mipHeight >> 2, 1);
                Int32 newMipDepth = Math::Max(mipDepth >> 2, 1);

                blit.dstSubresource.mipLevel = i;
                blit.dstOffsets[1] = { newMipWidth, newMipHeight, newMipDepth };

                vkCmdBlitImage(cmdBuf.handle, mImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, mImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

                TransitionLayout(cmdBuf, i - 1, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

                mipWidth = newMipWidth;
                mipHeight = newMipHeight;
                mipDepth = newMipDepth;
            }

            TransitionLayout(cmdBuf, mMipLevels - 1, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

            auto fence = mDevice.GetQueueManager().SubmitCommandBufferWithFence(cmdBuf);
            if(waitUntilFinished == true) {
                fence->Wait(10.0f);
                fence->Release();
                return nullptr;
            } else {
                return fence;
            }
        }

        Uint32 TextureVk::CalculateMipmapLevels(Uint32 width, Uint32 height)
        {
            Uint32 calculatedMipLevels = SCast(Uint32)(Math::Log2(SCast(float)(Math::Max(width, height))));

            return Math::Min(calculatedMipLevels, MaxMipLevels);
        }

        void TextureVk::TransitionLayout(VulkanCommandBuffer& cmdBuf, Int32 mipIndex, VkImageLayout newLayout, bool discardData)
        {
            CHECK(mCurrentLayouts[mipIndex] != newLayout, "Already transitioned layout({0}).", (int)newLayout);

            VkAccessFlags srcAccess, dstAccess;
            VkPipelineStageFlags srcPipelineStages, dstPipelineStages;

            GetImageBarrierInfos(mCurrentLayouts[mipIndex], srcAccess, srcPipelineStages);
            GetImageBarrierInfos(newLayout, dstAccess, dstPipelineStages);

            VkImageMemoryBarrier imgBarrier;
            imgBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            imgBarrier.pNext = nullptr;
            imgBarrier.srcAccessMask = srcAccess;
            imgBarrier.dstAccessMask = dstAccess;
            if(discardData == false) {
                imgBarrier.oldLayout = mCurrentLayouts[mipIndex];
            } else {
                imgBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            }
            imgBarrier.newLayout = newLayout;
            imgBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            imgBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            imgBarrier.image = mImage;
            imgBarrier.subresourceRange.aspectMask = mAspectMask;
            if(mipIndex == -1) {
                imgBarrier.subresourceRange.baseMipLevel = 0;
                imgBarrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
            } else {
                imgBarrier.subresourceRange.baseMipLevel = mipIndex;
                imgBarrier.subresourceRange.levelCount = 1;
            }
            imgBarrier.subresourceRange.baseArrayLayer = 0;
            imgBarrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

            vkCmdPipelineBarrier(cmdBuf.handle, srcPipelineStages, dstPipelineStages, 0, 0, nullptr, 0, nullptr, 1, &imgBarrier);

            mCurrentLayouts[mipIndex] = newLayout;
        }

        void TextureVk::GetImageBarrierInfos(VkImageLayout layout, VkAccessFlags& outAccessFlags, VkPipelineStageFlags& outPipelineStages)
        {
            switch(layout)
            {
                case VK_IMAGE_LAYOUT_UNDEFINED:
                case VK_IMAGE_LAYOUT_GENERAL:
                    outAccessFlags = 0;
                    outPipelineStages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                    return;

                case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                    outAccessFlags = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                    outPipelineStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                    return;

                case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
                    outAccessFlags = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                    outPipelineStages = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
                    return;

                case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
                case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
                case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL:
                case VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL:
                    outAccessFlags = VK_ACCESS_SHADER_READ_BIT;
                    outPipelineStages = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                    return;

                case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
                    outAccessFlags = VK_ACCESS_TRANSFER_READ_BIT;
                    outPipelineStages = VK_PIPELINE_STAGE_TRANSFER_BIT;
                    return;

                case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
                    outAccessFlags = VK_ACCESS_TRANSFER_WRITE_BIT;
                    outPipelineStages = VK_PIPELINE_STAGE_TRANSFER_BIT;
                    return;

                case VK_IMAGE_LAYOUT_PREINITIALIZED:
                    outAccessFlags = 0;
                    outPipelineStages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                    return;

                case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
                case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
                case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
                case VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL:
                    outAccessFlags = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                    outPipelineStages = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
                    return;

                case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
                    outAccessFlags = 0;
                    outPipelineStages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                    return;

                default:
                    ASSERTION_FAILED("Unknown VkImageLayout({0}).", (int)layout);
                    return;
            }
        }
    } // namespace rapi
} // namespace cube
