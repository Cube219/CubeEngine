#pragma once

#include "MetalHeader.h"

#include "GAPI_Viewport.h"

namespace cube
{
    namespace gapi
    {
        class MetalViewport : public Viewport
        {
        public:
            MetalViewport(const ViewportCreateInfo& createInfo) {}
            virtual ~MetalViewport() {}

            virtual void AcquireNextImage() override {}
            virtual void Present() override {}

            virtual void Resize(Uint32 width, Uint32 height) override {}
            virtual void SetVsync(bool vsync) override {}
        };
    } // namespace rapi
} // namespace cube
