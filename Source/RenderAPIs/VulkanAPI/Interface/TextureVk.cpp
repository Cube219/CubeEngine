#include "TextureVk.h"

#include "../VulkanDevice.h"
#include "../VulkanUtility.h"
#include "Core/Assertion.h"
#include "../VulkanTypeConversion.h"
#include "../VulkanDebug.h"
#include "Core/LogWriter.h"
#include "Utility/Math.h"

namespace cube
{
    namespace rapi
    {
        TextureVk::TextureVk(VulkanDevice& device, ResourceUsage usage, VkImageViewType type, FrameVector<TextureData> data,
            TextureFormat format, Uint32 width, Uint32 height, Uint32 depth, Uint32 arraySize,
            TextureBindTypeFlags bindTypeFlags, bool generateMipmaps, Uint32 samplesNum, const char* debugName) :
            mDevice(device),
            mMipLevelsInVk(1),
            mWidth(width), mHeight(height), mDepth(depth),
            mUsage(usage),
            mAspectMask(0),
            mDebugName(debugName)
        {
            if(generateMipmaps == true) {
                mMipLevelsInVk = CalculateMipmapLevels(width, height);
            } else {
                mMipLevelsInVk = Math::Min(SCast(Uint32)(data.size()), MaxMipLevels);
            }

            VkResult res;

            VkImageCreateInfo info;
            info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            info.pNext = nullptr;
            info.flags = 0;
            switch(type)
            {
                case VK_IMAGE_VIEW_TYPE_1D:
                // case VK_IMAGE_VIEW_TYPE_1D_ARRAY: // Unsupported
                    info.imageType = VK_IMAGE_TYPE_1D;
                    break;
                case VK_IMAGE_VIEW_TYPE_CUBE:
                case VK_IMAGE_VIEW_TYPE_CUBE_ARRAY:
                    info.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
                    // Fallthrough
                case VK_IMAGE_VIEW_TYPE_2D:
                case VK_IMAGE_VIEW_TYPE_2D_ARRAY:
                    info.imageType = VK_IMAGE_TYPE_2D;
                    break;
                case VK_IMAGE_VIEW_TYPE_3D:
                    info.imageType = VK_IMAGE_TYPE_3D;
                    break;
                default:
                    ASSERTION_FAILED("Invalid image type {}.", type);
                    break;
            }
            mFormat = TextureFormatToVkFormat(format);
            info.format = mFormat;
            CHECK(info.format != VK_FORMAT_UNDEFINED, "Texture format ({}) is not defined.", (int)format);
            info.extent = { width, height, depth };
            info.mipLevels = mMipLevelsInVk;
            info.arrayLayers = arraySize;
            switch(samplesNum)
            {
                case 1:  info.samples = VK_SAMPLE_COUNT_1_BIT;  break;
                case 2:  info.samples = VK_SAMPLE_COUNT_2_BIT;  break;
                case 4:  info.samples = VK_SAMPLE_COUNT_4_BIT;  break;
                case 8:  info.samples = VK_SAMPLE_COUNT_8_BIT;  break;
                case 16: info.samples = VK_SAMPLE_COUNT_16_BIT; break;
                case 32: info.samples = VK_SAMPLE_COUNT_32_BIT; break;
                case 64: info.samples = VK_SAMPLE_COUNT_64_BIT; break;
                default:
                    CUBE_LOG(LogType::Warning, "Unsupported sampler count ({}). Use sampler count 1 instead.", samplesNum);
                    info.samples = VK_SAMPLE_COUNT_1_BIT;
                    break;
            }
            info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            mCurrentLayouts.fill(VK_IMAGE_LAYOUT_UNDEFINED);
            info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
            if(bindTypeFlags.IsSet(TextureBindTypeFlag::RenderTarget)) {
                info.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
                mAspectMask |= VK_IMAGE_ASPECT_COLOR_BIT;
            }
            if(bindTypeFlags.IsSet(TextureBindTypeFlag::ShaderResource)) {
                info.usage |= VK_IMAGE_USAGE_STORAGE_BIT;
                mAspectMask |= VK_IMAGE_ASPECT_COLOR_BIT;
            }
            if(bindTypeFlags.IsSet(TextureBindTypeFlag::DepthStencil)) {
                info.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
                mAspectMask |= (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
            }
            info.queueFamilyIndexCount = 0;
            info.pQueueFamilyIndices = nullptr;
            info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            info.tiling = VK_IMAGE_TILING_OPTIMAL;

            res = vkCreateImage(device.GetHandle(), &info, nullptr, &mImage);
            CHECK_VK(res, "Failed to craete VkImage.");
            VULKAN_SET_OBJ_NAME(device.GetHandle(), mImage, debugName);

            // Allocate memory
            mAllocation = device.GetAllocator().Allocate(usage, info, &mImage);

            // Initialize data if it is existed
            if(data.size() > 0) {
                // Change the usage temporarily to enable map
                if(usage == ResourceUsage::Immutable) mUsage = ResourceUsage::Default;

                Uint32 numToCreate = mMipLevelsInVk;
                if(generateMipmaps) {
                    numToCreate = 1;
                }

                for(Uint32 i = 0; i < numToCreate; i++) {
                    void* p;
                    MapImpl(ResourceMapType::Write, i, p);
                    memcpy(p, data[i].pData, data[i].size);
                    UnmapImpl(i);
                }

                if(generateMipmaps) {
                    VulkanCommandBuffer uploadCmdBuf = mDevice.GetUploadCommandBuffer();
                    GenerateMipmaps(uploadCmdBuf);
                    mDevice.SubmitUploadCommandBuffer(uploadCmdBuf);
                }

                mUsage = usage;
            }
        }

        TextureVk::~TextureVk()
        {
            mDevice.GetAllocator().Free(mAllocation);
            vkDestroyImage(mDevice.GetHandle(), mImage, nullptr);
        }

        void TextureVk::MapImpl(ResourceMapType type, Uint32 mipIndex, void*& pMappedResource)
        {
            CHECK(mUsage != ResourceUsage::Immutable, "Cannot map immutable resource.");
            if(mUsage == ResourceUsage::Staging) {
                CHECK(type == ResourceMapType::Read, "Cannot map staging resource in write mode.");
            }

            if(mUsage == ResourceUsage::Dynamic) {
                CHECK(mipIndex == 0, "Cannot mapping more that 0 index in dynamic texture.")

                // Dynamic resource is always mapped
                pMappedResource = mAllocation.pMappedPtr;
                return;
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
            Uint64 bufSize = mipWidth * mipHeight * mipDepth * 4;
            mStagingBuffers[mipIndex] = mDevice.GetStagingManager().GetBuffer(bufSize, stagingBufType, mDebugName);

            if(type != ResourceMapType::Write) {
                VulkanCommandBuffer uploadCmdBuf = mDevice.GetUploadCommandBuffer();

                TransitionLayout(uploadCmdBuf, mipIndex, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

                VkBufferImageCopy region;
                region.bufferOffset = 0;
                region.bufferRowLength = 0;
                region.bufferImageHeight = 0;
                region.imageSubresource.aspectMask = mAspectMask;
                region.imageSubresource.mipLevel = mipIndex;
                region.imageSubresource.baseArrayLayer = 0; // TODO: Array 지원?
                region.imageSubresource.layerCount = 1;
                region.imageOffset = { 0, 0, 0 };
                region.imageExtent = { mipWidth, mipHeight, mipDepth };
                uploadCmdBuf.CopyImageToBuffer(mImage, mStagingBuffers[mipIndex].GetHandle(), region);

                TransitionLayout(uploadCmdBuf, mipIndex, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

                uploadCmdBuf.SetMemoryBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_HOST_BIT, VK_ACCESS_MEMORY_READ_BIT, VK_ACCESS_HOST_READ_BIT, mStagingBuffers[mipIndex].GetHandle(), bufSize);

                mDevice.SubmitUploadCommandBuffer(uploadCmdBuf, true);
            }

            pMappedResource = mStagingBuffers[mipIndex].GetMappedPtr();
        }

        void TextureVk::UnmapImpl(Uint32 mipIndex)
        {
            CHECK(mUsage != ResourceUsage::Immutable, "Cannot unmap immutable resource.");

            if(mUsage == ResourceUsage::Dynamic) {
                // Dynamic resource is always mapped
                return;
            }

            if(mStagingBuffers[mipIndex].IsValid() && mStagingBuffers[mipIndex].GetType() != VulkanStagingBuffer::Type::Read) {
                VulkanCommandBuffer uploadCmdBuf = mDevice.GetUploadCommandBuffer();

                TransitionLayout(uploadCmdBuf, mipIndex, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
                
                Uint32 mipWidth = Math::Max(mWidth >> mipIndex, 1u);
                Uint32 mipHeight = Math::Max(mHeight >> mipIndex, 1u);
                Uint32 mipDepth = Math::Max(mDepth >> mipIndex, 1u);

                VkBufferImageCopy region;
                region.bufferOffset = 0;
                region.bufferRowLength = 0;
                region.bufferImageHeight = 0;
                region.imageSubresource.aspectMask = mAspectMask;
                region.imageSubresource.mipLevel = mipIndex;
                region.imageSubresource.baseArrayLayer = 0;
                region.imageSubresource.layerCount = 1;
                region.imageOffset = { 0, 0, 0 };
                region.imageExtent = { mipWidth, mipHeight, mipDepth };

                uploadCmdBuf.CopyBufferToImage(mStagingBuffers[mipIndex].GetHandle(), mImage, region);

                TransitionLayout(uploadCmdBuf, mipIndex, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

                mDevice.SubmitUploadCommandBuffer(uploadCmdBuf);

                mDevice.GetStagingManager().ReleaseBuffer(std::move(mStagingBuffers[mipIndex]));
            }
        }

        Uint32 TextureVk::CalculateMipmapLevels(Uint32 width, Uint32 height)
        {
            Uint32 calculatedMipLevels = SCast(Uint32)(Math::Log2(SCast(float)(Math::Max(width, height))));

            return Math::Min(calculatedMipLevels, MaxMipLevels);
        }

        void TextureVk::GenerateMipmaps(VulkanCommandBuffer& cmdBuf)
        {
            VkPhysicalDevice gpu = mDevice.GetGPU();
            VkFormatProperties formatProps;
            vkGetPhysicalDeviceFormatProperties(gpu, mFormat, &formatProps);
            if((formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT) == 0) {
                CUBE_LOG(LogType::Warning, "Cannot generate mipmaps in format({}) in this gpu. Skip it.", (int)mFormat);
                return;
            }

            Int32 mipWidth = mWidth;
            Int32 mipHeight = mHeight;
            Int32 mipDepth = mDepth;

            TransitionLayout(cmdBuf, 0, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

            VkImageBlit blit;
            blit.srcSubresource.aspectMask = mAspectMask;
            blit.srcSubresource.baseArrayLayer = 0;
            blit.srcSubresource.layerCount = 1;
            blit.srcOffsets[0] = { 0, 0, 0 };

            blit.dstSubresource.aspectMask = mAspectMask;
            blit.dstSubresource.baseArrayLayer = 0;
            blit.dstSubresource.layerCount = 1;
            blit.dstOffsets[0] = { 0, 0, 0 };

            for(Uint32 i = 1; i < mMipLevelsInVk; i++) {
                TransitionLayout(cmdBuf, i, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

                blit.srcSubresource.mipLevel = i - 1;
                blit.srcOffsets[1] = { mipWidth, mipHeight, mipDepth };

                Int32 newMipWidth = Math::Max(mipWidth >> 2, 1);
                Int32 newMipHeight = Math::Max(mipHeight >> 2, 1);
                Int32 newMipDepth = Math::Max(mipDepth >> 2, 1);

                blit.dstSubresource.mipLevel = i;
                blit.dstOffsets[1] = { newMipWidth, newMipHeight, newMipDepth };

                vkCmdBlitImage(cmdBuf.GetHandle(), mImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, mImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

                TransitionLayout(cmdBuf, i - 1, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

                mipWidth = newMipWidth;
                mipHeight = newMipHeight;
                mipDepth =  newMipDepth;
            }

            TransitionLayout(cmdBuf, mMipLevelsInVk - 1, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
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

            vkCmdPipelineBarrier(cmdBuf.GetHandle(), srcPipelineStages, dstPipelineStages, 0, 0, nullptr, 0, nullptr, 1, &imgBarrier);

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

        Texture2DVk::Texture2DVk(VulkanDevice& device, const Texture2DCreateInfo& info) :
            Texture2D(info.usage, info.width, info.height, info.format, info.bindTypeFlags, 0/*Updated inside the constructor*/, info.samplesNum, info.debugName),
            TextureVk(device, info.usage, VK_IMAGE_VIEW_TYPE_2D, info.data,
                info.format, info.width, info.height, 1, 1,
                info.bindTypeFlags, info.generateMipmaps, info.samplesNum, info.debugName)
        {
            mMipLevels = GetMipLevelsInTextureVk();
        }
        Texture2DArrayVk::Texture2DArrayVk(VulkanDevice& device, const Texture2DArrayCreateInfo& info) :
            Texture2DArray(info.usage, info.arraySize, info.width, info.height, info.format, info.bindTypeFlags, 0/*Updated inside the constructor*/, info.samplesNum, info.debugName),
            TextureVk(device, info.usage, VK_IMAGE_VIEW_TYPE_2D_ARRAY, info.data,
                info.format, info.width, info.height, 1, info.arraySize,
                info.bindTypeFlags, info.generateMipmaps, info.samplesNum, info.debugName)
        {
            mMipLevels = GetMipLevelsInTextureVk();
        }
        Texture3DVk::Texture3DVk(VulkanDevice& device, const Texture3DCreateInfo& info) :
            Texture3D(info.usage, info.width, info.height, info.depth, info.format, info.bindTypeFlags, 0/*Updated inside the constructor*/, info.samplesNum, info.debugName),
            TextureVk(device, info.usage, VK_IMAGE_VIEW_TYPE_3D, info.data,
                info.format, info.width, info.height, info.depth, 1,
                info.bindTypeFlags, info.generateMipmaps, info.samplesNum, info.debugName)

        {
            mMipLevels = GetMipLevelsInTextureVk();
        }
        TextureCubeVk::TextureCubeVk(VulkanDevice& device, const TextureCubeCreateInfo& info) :
            TextureCube(info.usage, info.size, info.format, info.bindTypeFlags, 0/*Updated inside the constructor*/, info.samplesNum, info.debugName),
            TextureVk(device, info.usage, VK_IMAGE_VIEW_TYPE_CUBE, info.data,
                info.format, info.size, info.size, 1, 6,
                info.bindTypeFlags, info.generateMipmaps, info.samplesNum, info.debugName)
        {
            mMipLevels = GetMipLevelsInTextureVk();
        }
    } // namespace rapi
} // namespace cube
