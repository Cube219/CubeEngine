#pragma once

#include "VulkanAPIHeader.h"

namespace cube
{
    namespace rapi
    {
        class VulkanShader
        {
        public:
            VulkanShader(VulkanDevice& device);
            ~VulkanShader();

        private:
            VulkanDevice& mDevice;

            VkShaderModule mShaderModule;
        };
    } // namespace rapi
} // namespace cube
