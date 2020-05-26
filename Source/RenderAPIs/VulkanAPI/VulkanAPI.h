#pragma once

#include "VulkanAPIHeader.h"

#include "RenderAPIs/RenderAPI/RenderAPI.h"

namespace cube
{
    namespace rapi
    {
        extern "C" VK_API_EXPORT RenderAPI* CreateRenderAPI();

        class VK_API_EXPORT VulkanAPI : public RenderAPI
        {
        public:
            VulkanAPI() {}
            virtual ~VulkanAPI() {}

            virtual void Initialize() override;
            virtual void Shutdown() override;
        };
    } // namespace rapi
} // namespace cube
