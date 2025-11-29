#pragma once

#include "MetalHeader.h"

#include "GAPI_Metal.h"
#include "GAPI_SwapChain.h"

@interface CubeMTKView : MTKView <MTKViewDelegate>

@end

namespace cube
{
    namespace gapi
    {
        class MetalSwapChain : public SwapChain
        {
        public:
            MetalSwapChain(id<MTLDevice> device, CubeImGUIMTKView* imGUIView, const SwapChainCreateInfo& createInfo);
            virtual ~MetalSwapChain();

            virtual void AcquireNextImage() override;
            virtual void Present() override;

            virtual void Resize(Uint32 width, Uint32 height) override;
            virtual void SetVsync(bool vsync) override;

            virtual SharedPtr<TextureRTV> GetCurrentBackbufferRTV() const override;

        private:
            CubeMTKView* mView;
            MTLRenderPassDescriptor* mBackBufferDescriptor;
        };
    } // namespace rapi
} // namespace cube
