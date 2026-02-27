#pragma once

#include "DX12Header.h"

#include "GAPI_SwapChain.h"

#include "DX12APIObject.h"
#include "DX12DescriptorManager.h"
#include "DX12Fence.h"
#include "DX12MemoryAllocator.h"
#include "GAPI_DX12Texture.h"

#define MAX_BACKBUFFER_SIZE 5

namespace cube
{
    class DX12Device;
    class DX12TextureDSV;

    namespace gapi
    {
        class DX12BackbufferTexture : public DX12Texture
        {
        public:
            DX12BackbufferTexture(const TextureCreateInfo& createInfo, ComPtr<ID3D12Resource>& resource, DX12Device& device);
            virtual ~DX12BackbufferTexture();

            virtual void* Map() override;
            virtual void Unmap() override;

            virtual SharedPtr<TextureSRV> CreateSRV(const TextureSRVCreateInfo& createInfo) override;
            virtual SharedPtr<TextureUAV> CreateUAV(const TextureUAVCreateInfo& createInfo) override;
            virtual SharedPtr<TextureRTV> CreateRTV(const TextureRTVCreateInfo& createInfo) override;
            virtual SharedPtr<TextureDSV> CreateDSV(const TextureDSVCreateInfo& createInfo) override;

        private:
            ComPtr<ID3D12Resource> mResourceComPtr;
        };

        class CUBE_DX12_EXPORT DX12SwapChain : public SwapChain, public DX12APIObject
        {
        public:
            DX12SwapChain(IDXGIFactory2* factory, DX12Device& device, const SwapChainCreateInfo& createInfo);
            virtual ~DX12SwapChain();

            DX12SwapChain(const DX12SwapChain& other) = delete;
            DX12SwapChain& operator=(const DX12SwapChain& rhs) = delete;

            virtual void AcquireNextImage() override;
            virtual void Present() override;

            virtual void Resize(Uint32 width, Uint32 height) override;
            virtual void SetVsync(bool vsync) override;

            virtual SharedPtr<Texture> GetCurrentBackbuffer() const override
            {
                return mBackbufferTextures[mCurrentIndex];
            }

        private:
            void GetBackbuffers();
            void ClearBackbuffers();

            DX12Device& mDevice;

            ComPtr<IDXGISwapChain1> mSwapChain;
            ComPtr<IDXGISwapChain3> mSwapChain3;
            ElementFormat mFormat;
            DXGI_FORMAT mSwapChainFormat;
            Array<SharedPtr<DX12BackbufferTexture>, MAX_BACKBUFFER_SIZE> mBackbufferTextures;

            Uint32 mWidth;
            Uint32 mHeight;
            Uint32 mBackbufferCount;
            bool mVsyncSupported;
            bool mVsync;

            Uint32 mCurrentIndex;
            Array<Uint64, MAX_BACKBUFFER_SIZE> mFenceValues;
            DX12Fence mFence;
        };
    } // namespace gapi
} // namespace cube
