#pragma once

#include "../RenderAPIHeader.h"

namespace cube
{
    namespace rapi
    {
        struct SwapChainCreateInfo
        {
            Uint32 width;
            Uint32 height;
            bool vsync;

            TextureFormat colorBufferFormat;
            TextureFormat depthBufferFormat;

            Uint32 bufferCount;
        };

        class SwapChain
        {
        public:
            SwapChain() = default;
            virtual ~SwapChain() = default;

            virtual void AcquireNextImage() = 0;
            virtual void Present() = 0;

            virtual void Resize(Uint32 width, Uint32 height, bool vsync) = 0;
        };
    } // namespace rapi
} // namespace cube
