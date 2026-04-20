#pragma once

#include "MetalHeader.h"

#include "GAPI_Metal.h"
#include "GAPI_MetalTexture.h"
#include "GAPI_SwapChain.h"

@interface CubeMTKView : MTKView <MTKViewDelegate>

@end

namespace cube
{
    class MetalDevice;

    namespace gapi
    {
        class MetalBackbufferTexture : public MetalTexture
        {
        public:
            MetalBackbufferTexture(const TextureCreateInfo& createInfo, MetalDevice& device);
            virtual ~MetalBackbufferTexture();

            void UpdateDrawableTexture(id<MTLTexture> texture);

            virtual void* Map() override;
            virtual void Unmap() override;

            virtual SharedPtr<TextureSRV> CreateSRV(const TextureSRVCreateInfo& createInfo) override;
            virtual SharedPtr<TextureUAV> CreateUAV(const TextureUAVCreateInfo& createInfo) override;
            virtual SharedPtr<TextureRTV> CreateRTV(const TextureRTVCreateInfo& createInfo) override;
            virtual SharedPtr<TextureDSV> CreateDSV(const TextureDSVCreateInfo& createInfo) override;

        private:
            friend class MetalBackbufferRTV;

            Uint32 mViewReferenceCount;
        };

        class MetalBackbufferRTV : public MetalTextureRTV
        {
        public:
            MetalBackbufferRTV(const TextureRTVCreateInfo& createInfo, SharedPtr<MetalTexture> texture, MetalDevice& device);
            virtual ~MetalBackbufferRTV();

        private:
            MetalBackbufferTexture* mParentBackbufferTexture;
        };

        class MetalSwapChain : public SwapChain
        {
        public:
            MetalSwapChain(MetalDevice& device, CubeImGUIMTKView* imGUIView, const SwapChainCreateInfo& createInfo);
            virtual ~MetalSwapChain();

            virtual void AcquireNextImage() override;
            virtual void Present() override;

            virtual void Resize(Uint32 width, Uint32 height) override;
            virtual void SetVsync(bool vsync) override;

            virtual SharedPtr<Texture> GetCurrentBackbuffer() const override
            {
                return mBackbufferTexture;
            }

        private:
            MetalDevice& mDevice;

            CubeMTKView* mView;
            MTLRenderPassDescriptor* mBackbufferDescriptor;
            id<CAMetalDrawable> mCurrentDrawable;
            SharedPtr<MetalBackbufferTexture> mBackbufferTexture;
        };
    } // namespace rapi
} // namespace cube
