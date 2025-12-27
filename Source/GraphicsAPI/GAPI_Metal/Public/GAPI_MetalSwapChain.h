#pragma once

#include "MetalHeader.h"

#include "GAPI_Metal.h"
#include "GAPI_SwapChain.h"

@interface CubeMTKView : MTKView <MTKViewDelegate>

@end

namespace cube
{
    class MetalDevice;

    namespace gapi
    {
        class MetalSwapChain : public SwapChain
        {
        public:
            MetalSwapChain(MetalDevice& device, CubeImGUIMTKView* imGUIView, const SwapChainCreateInfo& createInfo);
            virtual ~MetalSwapChain();

            virtual void AcquireNextImage() override;
            virtual void Present() override;

            virtual void Resize(Uint32 width, Uint32 height) override;
            virtual void SetVsync(bool vsync) override;

            virtual SharedPtr<TextureRTV> GetCurrentBackbufferRTV() const override
            {
                return mCurrentBackbufferRTV;
            }

        private:
            MetalDevice& mDevice;

            CubeMTKView* mView;
            MTLRenderPassDescriptor* mBackbufferDescriptor;

            SharedPtr<TextureRTV> mCurrentBackbufferRTV;
        };
    } // namespace rapi
} // namespace cube
