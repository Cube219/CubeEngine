#pragma once

#include "VulkanAPIHeader.h"

namespace cube
{
    namespace rapi
    {
        class VulkanLayout
        {
        public:
            VulkanLayout();
            ~VulkanLayout();

        private:

            VkPipelineLayout mPipelineLayout;
        };
    } // namespace rapi
} // namespace cube
