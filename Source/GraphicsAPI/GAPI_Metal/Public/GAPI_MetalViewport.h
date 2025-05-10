#pragma once

#include "MetalHeader.h"

#include "GAPI_Metal.h"
#include "GAPI_Viewport.h"

@interface CubeMTKView : MTKView <MTKViewDelegate>

@end

namespace cube
{
    namespace gapi
    {
        class MetalViewport : public Viewport
        {
        public:
            MetalViewport(id<MTLDevice> device, CubeImGUIMTKView* imGUIView, const ViewportCreateInfo& createInfo);
            virtual ~MetalViewport();

            virtual void AcquireNextImage() override;
            virtual void Present() override;

            virtual void Resize(Uint32 width, Uint32 height) override;
            virtual void SetVsync(bool vsync) override;

        private:
            CubeMTKView* mView;
            MTLRenderPassDescriptor* mBackBufferDescriptor;
        };
    } // namespace rapi
} // namespace cube
