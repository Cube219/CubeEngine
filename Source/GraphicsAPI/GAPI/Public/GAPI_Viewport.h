#pragma once

#include "GAPIHeader.h"

namespace cube
{
    namespace gapi
    {
        struct ViewportCreateInfo
        {
            Uint32 width;
            Uint32 height;
            bool vsync;

            Uint32 backbufferCount;

            const char* debugName = "Unknown";
        };

        class Viewport
        {
        public:
            Viewport() = default;
            virtual ~Viewport() = default;

            virtual void AcquireNextImage() = 0;
            virtual void Present() = 0;

            virtual void Resize(Uint32 width, Uint32 height) = 0;
            virtual void SetVsync(bool vsync) = 0;
        };
    } // namespace gapi
} // namespace cube
