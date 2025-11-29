#include "GAPI_MetalSwapChain.h"

#include "MacOS/MacOSPlatform.h"
#include "MacOS/MacOSUtility.h"

@implementation CubeMTKView

- (void) drawInMTKView:(MTKView* ) view
{
}

- (void) mtkView:(MTKView* ) view drawableSizeWillChange:(CGSize)size
{
}

@end

namespace cube
{
    namespace gapi
    {
        MetalSwapChain::MetalSwapChain(id<MTLDevice> device, CubeImGUIMTKView* imGUIView, const SwapChainCreateInfo& createInfo)
        {
            platform::MacOSUtility::DispatchToMainThreadAndWait([this, device, imGUIView] {
                CubeWindow* window = platform::MacOSPlatform::GetWindow();
                NSRect windowFrame = window.contentView.bounds;
                mView = [[CubeMTKView alloc]
                    initWithFrame:windowFrame
                    device:device
                ];
                mView.delegate = mView;
                mView.paused = YES;
                mView.enableSetNeedsDisplay = YES;
                mView.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
                [window.contentView addSubview:mView positioned:NSWindowBelow relativeTo:imGUIView];
            });
        }

        MetalSwapChain::~MetalSwapChain()
        {
            platform::MacOSUtility::DispatchToMainThreadAndWait([this] {
                [mView release];
            });
        }

        void MetalSwapChain::AcquireNextImage()
        {
            do
            {
                mBackBufferDescriptor = mView.currentRenderPassDescriptor;
            } while (mBackBufferDescriptor == nil);
        }

        void MetalSwapChain::Present()
        {
            [mView.currentDrawable present];
            [mView draw];
        }

        void MetalSwapChain::Resize(Uint32 width, Uint32 height)
        {
            // TODO: It is needed?
        }

        void MetalSwapChain::SetVsync(bool vsync)
        {
            // TODO: It is possible?
        }

        SharedPtr<TextureRTV> MetalSwapChain::GetCurrentBackbufferRTV() const
        {
            return nullptr;
        }
    } // namespace gapi
} // namespace cube
