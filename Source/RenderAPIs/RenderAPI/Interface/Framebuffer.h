#pragma once

#include "../RenderAPIHeader.h"

namespace cube
{
    namespace rapi
    {
        struct FramebufferCreateInfo
        {
            RenderPass* pRenderPass = nullptr;

            const char* debugName = "";
        };

        class Framebuffer
        {
        public:
            Framebuffer() = default;
            virtual ~Framebuffer() = default;
        };
    } // namespace rapi
} // namespace cube
