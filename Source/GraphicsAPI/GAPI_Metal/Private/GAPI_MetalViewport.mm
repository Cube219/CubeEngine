#include "GAPI_MetalViewport.h"

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
        MetalViewport::MetalViewport(id<MTLDevice> device, CubeImGUIMTKView* imGUIView, const ViewportCreateInfo& createInfo)
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

        MetalViewport::~MetalViewport()
        {
            platform::MacOSUtility::DispatchToMainThreadAndWait([this] {
                [mView release];
            });
        }

        void MetalViewport::AcquireNextImage()
        {
            do
            {
                mBackBufferDescriptor = mView.currentRenderPassDescriptor;
            } while (mBackBufferDescriptor == nil);
        }

        void MetalViewport::Present()
        {
            [mView.currentDrawable present];
            [mView draw];
        }

        void MetalViewport::Resize(Uint32 width, Uint32 height)
        {
            // TODO: It is needed?
        }

        void MetalViewport::SetVsync(bool vsync)
        {
            // TODO: It is possible?
        }
    } // namespace gapi
} // namespace cube
