#include "GAPI_MetalSwapChain.h"

#include "Checker.h"
#include "Logger.h"
#include "MacOS/MacOSPlatform.h"
#include "MacOS/MacOSUtility.h"
#include "MetalDevice.h"
#include "MetalTypes.h"

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
            // Backbuffer has a one subresource.
            mSubresourceLayouts.push_back({ 0, 0 });
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

            if (mMTLTexture != nil)
            {
                mInfo.width = static_cast<Uint32>(mMTLTexture.width);
                mInfo.height = static_cast<Uint32>(mMTLTexture.height);
                mInfo.depth = 1;
                mInfo.arraySize = 1;
                mInfo.mipLevels = 1;
            }
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

        MetalSwapChain::MetalSwapChain(MetalDevice& device, CubeMetalView* view, const SwapChainCreateInfo& createInfo)
            : SwapChain(createInfo)
            , mDevice(device)
            , mView(view)
            , mDrawableLayer(nil)
            , mCurrentDrawable(nil)
        {
            CHECK(mView);

            platform::MacOSUtility::DispatchToMainThreadAndWait(
                [this, device = device.GetMTLDevice(),
                 width = createInfo.width, height = createInfo.height,
                 format = createInfo.backbufferFormat, vsync = createInfo.vsync]
                {
                mView.wantsLayer = YES;
                mView.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
                mView.frame = mView.superview != nil
                    ? mView.superview.bounds
                    : NSMakeRect(0.0, 0.0, width, height);

                mDrawableLayer = (CAMetalLayer*)mView.layer;
                CHECK(mDrawableLayer);

                [CATransaction begin];
                [CATransaction setDisableActions:YES];

                mDrawableLayer.device = device;
                mDrawableLayer.pixelFormat = GetMetalElementFormatInfo(format).pixelFormat;
                mDrawableLayer.framebufferOnly = NO;
                mDrawableLayer.opaque = YES;
                mDrawableLayer.maximumDrawableCount = 3;
                mDrawableLayer.displaySyncEnabled = vsync;

                [mView updateDrawableSize];

                [CATransaction commit];
            });

            TextureCreateInfo texCreateInfo = {
                .usage = ResourceUsage::GPUOnly,
                .textureInfo = {
                    .format = createInfo.backbufferFormat,
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
            mCurrentDrawable = nil;

            platform::MacOSUtility::DispatchToMainThreadAndWait([this] {
                [mView removeFromSuperview];
                mView = nil;
                mDrawableLayer = nil;
            });
        }

        bool MetalSwapChain::AcquireNextImage()
        {
            if (mDrawableLayer == nil)
            {
                return false;
            }

            id<CAMetalDrawable> drawable = [mDrawableLayer nextDrawable];
            if (drawable == nil)
            {
                return false;
            }

            mCurrentDrawable = drawable;
            mBackbufferTexture->UpdateDrawableTexture(mCurrentDrawable.texture);

            return true;
        }

        void MetalSwapChain::Present()
        {
            if (mCurrentDrawable == nil)
            {
                return;
            }

            id<MTLCommandBuffer> commandBuffer = [mDevice.GetMainCommandQueue() commandBuffer];
            commandBuffer.label = @"Present command buffer";
            [commandBuffer presentDrawable:mCurrentDrawable];
            [commandBuffer commit];

            mCurrentDrawable = nil;
        }

        void MetalSwapChain::Resize(Uint32 width, Uint32 height)
        {
            if (width == 0 || height == 0)
            {
                return;
            }

            platform::MacOSUtility::DispatchToMainThreadAndWait([this, width, height] {
                [CATransaction begin];
                [CATransaction setDisableActions:YES];

                mView.frame = mView.superview != nil
                    ? mView.superview.bounds
                    : NSMakeRect(0.0, 0.0, width, height);
                [mView updateDrawableSize];

                [CATransaction commit];
            });
        }

        void MetalSwapChain::SetVsync(bool vsync)
        {
            platform::MacOSUtility::DispatchToMainThreadAndWait([this, vsync] {
                if (mDrawableLayer != nil)
                {
                    mDrawableLayer.displaySyncEnabled = vsync;
                }
            });
        }
    } // namespace gapi
} // namespace cube
