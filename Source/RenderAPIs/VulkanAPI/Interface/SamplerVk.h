#pragma once

#include "../VulkanAPIHeader.h"

#include "RenderAPIs/RenderAPI/Interface/Sampler.h"

namespace cube
{
    namespace rapi
    {
        VkFilter ConvertToVkFilter(FilterType filterType);
        VkSamplerMipmapMode ConvertToVkSamplerMipmapMode(FilterType filterType);
        VkSamplerAddressMode ConvertToVkSamplerAddressMode(AddressMode addressMode);
        VkBorderColor ConvertToVkBorderColor(const float color[4]);

        class SamplerVk : public Sampler
        {
        public:
            SamplerVk(VulkanDevice& device, const SamplerCreateInfo& info);
            virtual ~SamplerVk();

            VkSampler GetHandle() const { return mSampler; }

        private:
            VulkanDevice& mDevice;

            VkSampler mSampler;
        };
    } // namespace rapi
    
} // namespace cube
