#include "GAPI_DX12SwapChain.h"


#include "DX12Device.h"
#include "DX12Utility.h"
#include "GAPI_DX12Texture.h"
#include "Windows/WindowsPlatform.h"

namespace cube
{
    namespace gapi
    {
        DX12BackbufferTexture::DX12BackbufferTexture(const TextureCreateInfo& createInfo, ComPtr<ID3D12Resource>& resource, DX12Device& device)
            : DX12Texture(createInfo, resource.Get(), device)
            , mResourceComPtr(resource)
        {
        }

        DX12BackbufferTexture::~DX12BackbufferTexture()
        {
        }

        void* DX12BackbufferTexture::Map()
        {
            NO_ENTRY_FORMAT("Cannot map backbuffer texture.");
            return nullptr;
        }

        void DX12BackbufferTexture::Unmap()
        {
            NO_ENTRY_FORMAT("Cannot unmap backbuffer texture.");
        }

        SharedPtr<TextureSRV> DX12BackbufferTexture::CreateSRV(const TextureSRVCreateInfo& createInfo)
        {
            NO_ENTRY_FORMAT("Cannot create SRV from backbuffer texture");
            return nullptr;
        }

        SharedPtr<TextureUAV> DX12BackbufferTexture::CreateUAV(const TextureUAVCreateInfo& createInfo)
        {
            NO_ENTRY_FORMAT("Cannot create UAV from backbuffer texture");
            return nullptr;
        }

        SharedPtr<TextureRTV> DX12BackbufferTexture::CreateRTV(const TextureRTVCreateInfo& createInfo)
        {
            return DX12Texture::CreateRTV(createInfo);
        }

        SharedPtr<TextureDSV> DX12BackbufferTexture::CreateDSV(const TextureDSVCreateInfo& createInfo)
        {
            NO_ENTRY_FORMAT("Cannot create DSV from backbuffer texture");
            return nullptr;
        }

        DX12SwapChain::DX12SwapChain(IDXGIFactory2* factory, DX12Device& device, const SwapChainCreateInfo& createInfo)
            : mDevice(device)
            , mFence(device)
        {
            mWidth = createInfo.width;
            mHeight = createInfo.height;
            mVsync = createInfo.vsync;
            // Not implemented custom backbuffer size
            // It should consider command list manager and imgui re-initialization
            mBackbufferCount = 2; // createInfo.backbufferCount;

            mVsyncSupported = true; // TODO
            mCurrentIndex = 0;
            mFenceValues = {};

            mFormat = ElementFormat::RGBA8_UNorm;
            mSwapChainFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

            DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
            swapChainDesc.BufferCount = mBackbufferCount;
            swapChainDesc.Width = mWidth;
            swapChainDesc.Height = mHeight;
            swapChainDesc.Format = mSwapChainFormat;
            swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
            swapChainDesc.SampleDesc.Count = 1;

            CHECK_HR(factory->CreateSwapChainForHwnd(
                device.GetQueueManager().GetMainQueue(),
                platform::WindowsPlatform::GetWindow(),
                &swapChainDesc,
                nullptr,
                nullptr,
                &mSwapChain
                ));
            SET_DEBUG_NAME(mSwapChain, createInfo.debugName);
            if (HRESULT res = mSwapChain.As(&mSwapChain3); res != S_OK)
            {
                CUBE_LOG(Warning, DX12, "Cannot cast IDXGISwapChain3. Frame buffering cannot be used. (HR: {0})", res);
            }

            mFence.Initialize(CUBE_T("SwapChainFence"));

            GetBackbuffers();
        }

        DX12SwapChain::~DX12SwapChain()
        {
            mFence.Shutdown();

            ClearBackbuffers();

            mSwapChain3 = nullptr;
            mSwapChain = nullptr;
        }

        void DX12SwapChain::AcquireNextImage()
        {
            Uint64 currentFenceValue = mFenceValues[mCurrentIndex];

            if (mSwapChain3)
            {
                mCurrentIndex = mSwapChain3->GetCurrentBackBufferIndex();
            }
            mFence.Wait(currentFenceValue);

            mFenceValues[mCurrentIndex] = currentFenceValue + 1;
        }

        void DX12SwapChain::Present()
        {
            mSwapChain->Present(mVsync ? 1 : 0, 0);

            mFence.Signal(mDevice.GetQueueManager().GetMainQueue(), mFenceValues[mCurrentIndex]);
        }

        void DX12SwapChain::Resize(Uint32 width, Uint32 height)
        {
            ClearBackbuffers();

            mWidth = width;
            mHeight = height;

            CHECK_HR(mSwapChain->ResizeBuffers(mBackbufferCount, mWidth, mHeight, mSwapChainFormat, 0));

            GetBackbuffers();
        }

        void DX12SwapChain::SetVsync(bool vsync)
        {
            mVsync = vsync;
        }

        void DX12SwapChain::GetBackbuffers()
        {
            ClearBackbuffers();

            TextureCreateInfo backbufferCreateInfo = {
                .usage = ResourceUsage::GPUOnly,
                .format = mFormat,
                .type = TextureType::Texture2D,
                .flags = TextureFlag::RenderTarget,
                .width = mWidth,
                .height = mHeight,
                .mipLevels = 1
            };
            for (Uint32 i = 0; i < mBackbufferCount; ++i)
            {
                ComPtr<ID3D12Resource> backbuffer;
                mSwapChain.Get()->GetBuffer(i, IID_PPV_ARGS(&backbuffer));

                FrameString debugName = Format<FrameString>(CUBE_T("SwapChainBackbuffer_{0}"), i);
                backbufferCreateInfo.debugName = debugName;
                mBackbufferTextures[i] = std::make_shared<DX12BackbufferTexture>(backbufferCreateInfo, backbuffer, mDevice);
            }
        }

        void DX12SwapChain::ClearBackbuffers()
        {
            for (SharedPtr<DX12BackbufferTexture>& texture : mBackbufferTextures)
            {
                texture = nullptr;
            }
        }
    } // namespace gapi
} // namespace cube
