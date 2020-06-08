#include "TextureVk.h"

#include "../VulkanDevice.h"
#include "../VulkanUtility.h"
#include "Core/Assertion.h"
#include "../VulkanTypeConversion.h"
#include "../VulkanDebug.h"

namespace cube
{
    namespace rapi
    {
        TextureVk::TextureVk(VulkanDevice& device, ResourceUsage resUsage, VkImageViewType type, TextureFormat format, Uint32 width, Uint32 height, Uint32 depth, Uint32 arraySize, TextureBindTypeFlags bindTypeFlags, Uint32 mipLevels, Uint32 samplesNum, const char* debugName) :
            mDevice(device)
        {
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
                    // Fallback
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
            info.format = TextureFormatToVkFormat(format);
            CHECK(info.format != VK_FORMAT_UNDEFINED, "Texture format ({}) is not defined.", (int)format);
            info.extent = { width, height, depth };
            info.mipLevels = mipLevels;
            info.arrayLayers = arraySize;
            switch(samplesNum)
            {
                case 1:
                    info.samples = VK_SAMPLE_COUNT_1_BIT;
                    break;
                case 2:
                    info.samples = VK_SAMPLE_COUNT_2_BIT;
                    break;
                case 4:
                    info.samples = VK_SAMPLE_COUNT_4_BIT;
                    break;
                case 8:
                    info.samples = VK_SAMPLE_COUNT_8_BIT;
                    break;
                case 16:
                    info.samples = VK_SAMPLE_COUNT_16_BIT;
                    break;
                case 32:
                    info.samples = VK_SAMPLE_COUNT_32_BIT;
                    break;
                case 64:
                    info.samples = VK_SAMPLE_COUNT_64_BIT;
                    break;
                default:
                    CUBE_LOG(LogType::Warning, "Unsupported sampler count ({}). Use sampler count 1 instead.");
                    info.samples = VK_SAMPLE_COUNT_1_BIT;
                    break;
            }
            info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
            if(bindTypeFlags.IsSet(TextureBindTypeFlag::RenderTarget)) {
                info.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
            }
            if(bindTypeFlags.IsSet(TextureBindTypeFlag::ShaderResource)) {
                info.usage |= VK_IMAGE_USAGE_STORAGE_BIT;
            }
            if(bindTypeFlags.IsSet(TextureBindTypeFlag::DepthStencil)) {
                info.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            }
            info.queueFamilyIndexCount = 0;
            info.pQueueFamilyIndices = nullptr;
            info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            info.tiling = VK_IMAGE_TILING_OPTIMAL;

            res = vkCreateImage(device.GetDevice(), &info, nullptr, &mImage);
            CHECK_VK(res, "Failed to craete VkImage.");
            VULKAN_SET_OBJ_NAME(device.GetDevice(), mImage, debugName);

            // Allocate memory
            mAllocation = device.GetAllocator().Allocate(resUsage, info, &mImage);
        }

        TextureVk::~TextureVk()
        {
            mDevice.GetAllocator().Free(mAllocation);
            vkDestroyImage(mDevice.GetDevice(), mImage, nullptr);
        }

        Texture2DVk::Texture2DVk(VulkanDevice& device, const Texture2DCreateInfo& info) :
            Texture2D(info.usage, info.width, info.height, info.format, info.bindTypeFlags, info.mipLevels, info.samplesNum, info.debugName),
            TextureVk(device, info.usage, VK_IMAGE_VIEW_TYPE_2D, info.format, info.width, info.height, 1, 1, info.bindTypeFlags, info.mipLevels, info.samplesNum, info.debugName)
        {}
        Texture2DArrayVk::Texture2DArrayVk(VulkanDevice& device, const Texture2DArrayCreateInfo& info) :
            Texture2DArray(info.usage, info.arraySize, info.width, info.height, info.format, info.bindTypeFlags, info.mipLevels, info.samplesNum, info.debugName),
            TextureVk(device, info.usage, VK_IMAGE_VIEW_TYPE_2D_ARRAY, info.format, info.width, info.height, 1, info.arraySize, info.bindTypeFlags, info.mipLevels, info.samplesNum, info.debugName)
        {}
        Texture3DVk::Texture3DVk(VulkanDevice& device, const Texture3DCreateInfo& info) :
            Texture3D(info.usage, info.width, info.height, info.depth, info.format, info.bindTypeFlags, info.mipLevels, info.samplesNum, info.debugName),
            TextureVk(device, info.usage, VK_IMAGE_VIEW_TYPE_3D, info.format, info.width, info.height, info.depth, 1, info.bindTypeFlags, info.mipLevels, info.samplesNum, info.debugName)

        {}
        TextureCubeVk::TextureCubeVk(VulkanDevice& device, const TextureCubeCreateInfo& info) :
            TextureCube(info.usage, info.size, info.format, info.bindTypeFlags, info.mipLevels, info.samplesNum, info.debugName),
            TextureVk(device, info.usage, VK_IMAGE_VIEW_TYPE_CUBE, info.format, info.size, info.size, 1, 6, info.bindTypeFlags, info.mipLevels, info.samplesNum, info.debugName)
        {}
    } // namespace rapi
} // namespace cube
