#include "GAPI_MetalSwapChain.h"

#include "Checker.h"
#include "Logger.h"
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
        // MetalBackbufferTexture

        MetalBackbufferTexture::MetalBackbufferTexture(const TextureCreateInfo& createInfo, MetalDevice& device)
            : Texture(createInfo)
            , mDevice(device)
            , mDrawableTexture(nil)
        {
        }

        MetalBackbufferTexture::~MetalBackbufferTexture()
        {
        }

        void* MetalBackbufferTexture::Map()
        {
            CHECK_FORMAT(false, "Cannot map backbuffer texture.");
            return nullptr;
        }

        void MetalBackbufferTexture::Unmap()
        {
            CHECK_FORMAT(false, "Cannot unmap backbuffer texture.");
        }

        SharedPtr<TextureSRV> MetalBackbufferTexture::CreateSRV(const TextureSRVCreateInfo& createInfo)
        {
            CHECK_FORMAT(false, "Cannot create SRV for backbuffer texture.");
            return nullptr;
        }

        SharedPtr<TextureUAV> MetalBackbufferTexture::CreateUAV(const TextureUAVCreateInfo& createInfo)
        {
            CHECK_FORMAT(false, "Cannot create UAV for backbuffer texture.");
            return nullptr;
        }

        SharedPtr<TextureRTV> MetalBackbufferTexture::CreateRTV(const TextureRTVCreateInfo& createInfo)
        {
            CHECK(mDrawableTexture);
            
            if (createInfo.mipLevel != 0 || createInfo.firstArrayIndex != 0 || createInfo.firstDepthIndex != 0)
            {
                CUBE_LOG(Warning, MetalSwapChain, "Try to create RTV for backbuffer which is not base view (mipLevel=0, arrayIndex=0, depthIndex=0). Ignore that and use base view.");
            }
            return std::make_shared<MetalTextureRTV>(mDrawableTexture, mDevice);
        }

        SharedPtr<TextureDSV> MetalBackbufferTexture::CreateDSV(const TextureDSVCreateInfo& createInfo)
        {
            CHECK_FORMAT(false, "Cannot create DSV for backbuffer texture.");
            return nullptr;
        }

        // MetalSwapChain

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

            TextureCreateInfo texInfo;
            texInfo.usage = ResourceUsage::GPUOnly;
            texInfo.format = ElementFormat::RGBA8_UNorm;
            texInfo.type = TextureType::Texture2D;
            texInfo.flags = TextureFlag::RenderTarget;
            texInfo.width = createInfo.width;
            texInfo.height = createInfo.height;
            texInfo.debugName = CUBE_T("Backbuffer");
            mBackbufferTexture = std::make_shared<MetalBackbufferTexture>(texInfo, mDevice);
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
                mBackbufferDescriptor = mView.currentRenderPassDescriptor;
            } while (mBackbufferDescriptor == nil);

            mBackbufferTexture->UpdateDrawableTexture(mView.currentDrawable.texture);
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
