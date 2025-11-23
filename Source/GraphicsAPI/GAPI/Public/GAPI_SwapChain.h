#pragma once

#include "GAPIHeader.h"

namespace cube
{
    namespace gapi
    {
        class TextureRTV;

        struct SwapChainCreateInfo
        {
            Uint32 width;
            Uint32 height;
            bool vsync;

            Uint32 backbufferCount;

            StringView debugName;
        };

        class SwapChain
        {
        public:
            SwapChain() = default;
            virtual ~SwapChain() = default;

            virtual void AcquireNextImage() = 0;
            virtual void Present() = 0;

            virtual void Resize(Uint32 width, Uint32 height) = 0;
            virtual void SetVsync(bool vsync) = 0;

            virtual SharedPtr<TextureRTV> GetCurrentBackbufferRTV() const = 0;
        };
    } // namespace gapi
} // namespace cube
