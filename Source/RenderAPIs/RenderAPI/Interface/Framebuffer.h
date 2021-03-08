#pragma once

#include "../RenderAPIHeader.h"

namespace cube
{
    namespace rapi
    {
        struct FramebufferCreateInfo
        {
            RenderPass* pRenderPass = nullptr;
            Uint32 numAttachments;
            TextureView** ppAttachments;

            Uint32 width;
            Uint32 height;
            Uint32 arrayStartIndex;

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
