#include "GAPI_MetalSwapChain.h"

#include "GAPI_MetalTexture.h"
#include "MacOS/MacOSPlatform.h"
#include "MacOS/MacOSUtility.h"
#include "MetalDevice.h"

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
        MetalSwapChain::MetalSwapChain(MetalDevice& device, CubeImGUIMTKView* imGUIView, const SwapChainCreateInfo& createInfo)
            : mDevice(device)
        {
            platform::MacOSUtility::DispatchToMainThreadAndWait(
                [this, device = device.GetMTLDevice(), imGUIView, width = createInfo.width, height = createInfo.height]
                {
                CubeWindow* window = platform::MacOSPlatform::GetWindow();
                NSRect windowFrame = window.contentView.bounds;
                mView = [[CubeMTKView alloc]
                    initWithFrame:windowFrame
                    device:device
                ];
                mView.delegate = mView;
                mView.paused = YES;
                mView.enableSetNeedsDisplay = YES;
                mView.autoResizeDrawable = NO;
                CGSize size;
                size.width = width;
                size.height = height;
                mView.drawableSize = size;
                mView.colorPixelFormat = MTLPixelFormatRGBA8Unorm;
                [window.contentView addSubview:mView positioned:NSWindowBelow relativeTo:imGUIView];
            });
        }

        MetalSwapChain::~MetalSwapChain()
        {
            mCurrentBackbufferRTV = nullptr;

            platform::MacOSUtility::DispatchToMainThreadAndWait([this] {
                [mView release];
            });
        }

        void MetalSwapChain::AcquireNextImage()
        {
            mCurrentBackbufferRTV = nullptr;

            do
            {
                mBackbufferDescriptor = mView.currentRenderPassDescriptor;
            } while (mBackbufferDescriptor == nil);

            id<MTLTexture> backbuffer = mBackbufferDescriptor.colorAttachments[0].texture;
            mCurrentBackbufferRTV = std::make_shared<MetalTextureRTV>(backbuffer, mDevice);
        }

        void MetalSwapChain::Present()
        {
            [mView.currentDrawable present];
            [mView draw];
        }

        void MetalSwapChain::Resize(Uint32 width, Uint32 height)
        {
            CGSize newSize;
            newSize.width = width;
            newSize.height = height;
            [mView setDrawableSize:newSize];
        }

        void MetalSwapChain::SetVsync(bool vsync)
        {
            // TODO: It is possible?
        }
    } // namespace gapi
} // namespace cube
