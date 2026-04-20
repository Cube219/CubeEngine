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
            : MetalTexture(createInfo, device, true)
            , mViewReferenceCount(0)
        {
            mFromExisted = true;
        }

        MetalBackbufferTexture::~MetalBackbufferTexture()
        {
        }

        void* MetalBackbufferTexture::Map()
        {
            NO_ENTRY_FORMAT("Cannot map backbuffer texture.");
            return nullptr;
        }

        void MetalBackbufferTexture::Unmap()
        {
            NO_ENTRY_FORMAT("Cannot unmap backbuffer texture.");
        }

        SharedPtr<TextureSRV> MetalBackbufferTexture::CreateSRV(const TextureSRVCreateInfo& createInfo)
        {
            NO_ENTRY_FORMAT("Cannot create SRV for backbuffer texture.");
            return nullptr;
        }

        SharedPtr<TextureUAV> MetalBackbufferTexture::CreateUAV(const TextureUAVCreateInfo& createInfo)
        {
            NO_ENTRY_FORMAT("Cannot create UAV for backbuffer texture.");
            return nullptr;
        }

        SharedPtr<TextureRTV> MetalBackbufferTexture::CreateRTV(const TextureRTVCreateInfo& createInfo)
        {
            return std::make_shared<MetalBackbufferRTV>(createInfo, shared_from_this(), mDevice);
        }

        SharedPtr<TextureDSV> MetalBackbufferTexture::CreateDSV(const TextureDSVCreateInfo& createInfo)
        {
            CHECK_FORMAT(false, "Cannot create DSV for backbuffer texture.");
            return nullptr;
        }

        void MetalBackbufferTexture::UpdateDrawableTexture(id<MTLTexture> texture)
        {
            CHECK_FORMAT(mViewReferenceCount == 0, "You must release all related views before update drawable texture.");

            mMTLTexture = texture;
        }

        MetalBackbufferRTV::MetalBackbufferRTV(const TextureRTVCreateInfo& createInfo, SharedPtr<MetalTexture> texture, MetalDevice& device)
            : MetalTextureRTV(createInfo, texture, device)
            , mParentBackbufferTexture(dynamic_cast<MetalBackbufferTexture*>(texture.get()))
        {
            CHECK(mParentBackbufferTexture);
            mParentBackbufferTexture->mViewReferenceCount++;
        }

        MetalBackbufferRTV::~MetalBackbufferRTV()
        {
            mParentBackbufferTexture->mViewReferenceCount--;
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
                mView.enableSetNeedsDisplay = NO;
                mView.autoResizeDrawable = NO;
                mView.framebufferOnly = NO;
                CGSize size;
                size.width = width;
                size.height = height;
                mView.drawableSize = size;
                mView.colorPixelFormat = MTLPixelFormatRGBA8Unorm;
                [window.contentView addSubview:mView positioned:NSWindowBelow relativeTo:imGUIView];
            });

            TextureCreateInfo texCreateInfo = {
                .usage = ResourceUsage::GPUOnly,
                .textureInfo = {
                    .format = ElementFormat::RGBA8_UNorm,
                    .type = TextureType::Texture2D,
                    .flags = TextureFlag::RenderTarget,
                    .width = createInfo.width,
                    .height = createInfo.height
                },
                .debugName = CUBE_T("Backbuffer")
            };
            mBackbufferTexture = std::make_shared<MetalBackbufferTexture>(texCreateInfo, mDevice);
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

            mCurrentDrawable = mView.currentDrawable;
            mBackbufferTexture->UpdateDrawableTexture(mCurrentDrawable.texture);
        }

        void MetalSwapChain::Present()
        {
            id<MTLCommandBuffer> commandBuffer = [mDevice.GetMainCommandQueue() commandBuffer];
            [commandBuffer presentDrawable:mCurrentDrawable];
            [commandBuffer commit];
            mCurrentDrawable = nil;

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
