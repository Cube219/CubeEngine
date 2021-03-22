#include "SamplerVk.h"

#include "../VulkanDebug.h"
#include "../VulkanDevice.h"

namespace cube
{
    namespace rapi
    {
        VkFilter ConvertToVkFilter(FilterType filterType)
        {
            switch(filterType) {
                case FilterType::Point:  return VK_FILTER_NEAREST;
                case FilterType::Linear: return VK_FILTER_LINEAR;
                default:
                    ASSERTION_FAILED("Unknown FilterType ({})", filterType);
            }
            return VK_FILTER_NEAREST;
        }

        VkSamplerMipmapMode ConvertToVkSamplerMipmapMode(FilterType filterType)
        {
            switch(filterType) {
                case FilterType::Point:  return VK_SAMPLER_MIPMAP_MODE_NEAREST;
                case FilterType::Linear: return VK_SAMPLER_MIPMAP_MODE_LINEAR;
                default:
                    ASSERTION_FAILED("Unknown FilterType ({})", filterType);
            }
            return VK_SAMPLER_MIPMAP_MODE_NEAREST;
        }

        VkSamplerAddressMode ConvertToVkSamplerAddressMode(AddressMode addressMode)
        {
            switch(addressMode) {
                case AddressMode::Wrap: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
                case AddressMode::BorderColor: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
                case AddressMode::Clamp: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
                case AddressMode::Mirror: return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
                case AddressMode::MirrorOnce: return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
                default:
                    ASSERTION_FAILED("Unknown AddressMode ({0})", addressMode);
            }
            return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        }

        VkBorderColor ConvertToVkBorderColor(const float color[4])
        {
            if(color[0] == 0.0f && color[1] == 0.0f && color[2] == 0.0f && color[3] == 0.0f)
                return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
            else if(color[0] == 0.0f && color[1] == 0.0f && color[2] == 0.0f && color[3] == 1.0f)
                return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
            else if(color[0] == 1.0f && color[1] == 1.0f && color[2] == 1.0f && color[3] == 1.0f)
                return VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

            CUBE_LOG(LogType::Warning, "In vulkan, border color must be transparent black(0, 0, 0, 0), opaque black(0, 0, 0, 1) or opaque white(1, 1, 1, 1).");
            CUBE_LOG(LogType::Warning, "So the border color ({}, {}, {}, {}) has been replaced opqeue black(0, 0, 0, 1).", color[0], color[1], color[2], color[3]);

            return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
        }

        SamplerVk::SamplerVk(VulkanDevice& device, const SamplerCreateInfo& info) :
            mDevice(device)
        {
            VkResult res;

            VkSamplerCreateInfo samplerCreateInfo;
            samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            samplerCreateInfo.pNext = nullptr;
            samplerCreateInfo.flags = 0;
            samplerCreateInfo.magFilter = ConvertToVkFilter(info.magFilter);
            samplerCreateInfo.minFilter = ConvertToVkFilter(info.minFilter);;
            samplerCreateInfo.mipmapMode = ConvertToVkSamplerMipmapMode(info.mipFilter);
            samplerCreateInfo.addressModeU = ConvertToVkSamplerAddressMode(info.addressU);
            samplerCreateInfo.addressModeV = ConvertToVkSamplerAddressMode(info.addressV);
            samplerCreateInfo.addressModeW = ConvertToVkSamplerAddressMode(info.addressW);
            samplerCreateInfo.mipLodBias = info.mipLODBias;
            samplerCreateInfo.anisotropyEnable = (info.maxAnisotropy > 0) ? VK_TRUE : VK_FALSE;
            samplerCreateInfo.maxAnisotropy = SCast(float)(info.maxAnisotropy);
            samplerCreateInfo.compareEnable = VK_FALSE; // TODO: 차후 구현
            samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
            samplerCreateInfo.minLod = info.minLOD;
            samplerCreateInfo.maxLod = info.maxLOD;
            samplerCreateInfo.borderColor = ConvertToVkBorderColor(info.borderColor);
            samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;

            res = vkCreateSampler(mDevice.GetHandle(), &samplerCreateInfo, nullptr, &mSampler);
            CHECK_VK(res, "Failed to create VkSampler.");
            VULKAN_SET_OBJ_NAME(mDevice.GetHandle(), mSampler, info.debugName);
        }

        SamplerVk::~SamplerVk()
        {
            vkDestroySampler(mDevice.GetHandle(), mSampler, nullptr);
        }
    } // namespace rapi
} // namespace cube
