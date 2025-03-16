#include "GAPI_DX12Viewport.h"

#include <dxgidebug.h>

#include "Windows/WindowsPlatform.h"

#include "DX12Device.h"
#include "DX12Utility.h"

namespace cube
{
    namespace gapi
    {
        DX12Viewport::DX12Viewport(IDXGIFactory2* factory, DX12Device& device, const ViewportCreateInfo& createInfo) :
            mDevice(device),
            mFence(device)
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
                CUBE_LOG(LogType::Warning, DX12, "Cannot cast IDXGISwapChain3. Frame buffering cannot be used. (HR: {0})", res);
            }

            // Backbuffer / RTV
            D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
            rtvHeapDesc.NumDescriptors = mBackbufferCount;
            rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
            CHECK_HR(device.GetDevice()->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&mRTVDescriptorHeap)));
            mRTVDescriptorSize = device.GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

            GetBackbuffers();

            mFence.Initialize();
        }

        DX12Viewport::~DX12Viewport()
        {
            // TODO: Implement WaitCPU in QueueManager
            // mDevice.GetQueueManager().GetMainQueue()->Signal(mFence.Get(), mFenceValues[mCurrentIndex] + 5);
            // CHECK_HR(mFence->SetEventOnCompletion(mFenceValues[mCurrentIndex] + 5, mFenceEvent));
            // WaitForSingleObjectEx(mFenceEvent, INFINITE, FALSE);

            mFence.Shutdown();

            ClearBackbuffers();

            mRTVDescriptorHeap = nullptr;

            mSwapChain3 = nullptr;
            mSwapChain = nullptr;
        }

        void DX12Viewport::AcquireNextImage()
        {
            Uint64 currentFenceValue = mFenceValues[mCurrentIndex];

            if (mSwapChain3)
            {
                mCurrentIndex = mSwapChain3->GetCurrentBackBufferIndex();
            }
            mFence.Wait(currentFenceValue);

            mFenceValues[mCurrentIndex] = currentFenceValue + 1;
        }

        void DX12Viewport::Present()
        {
            mSwapChain->Present(mVsync ? 1 : 0, 0);

            mFence.Signal(mDevice.GetQueueManager().GetMainQueue(), mFenceValues[mCurrentIndex]);
        }

        void DX12Viewport::Resize(Uint32 width, Uint32 height)
        {
            ClearBackbuffers();
            
            mWidth = width;
            mHeight = height;
            
            CHECK_HR(mSwapChain->ResizeBuffers(mBackbufferCount, mWidth, mHeight, mSwapChainFormat, 0));
            
            GetBackbuffers();
        }

        void DX12Viewport::SetVsync(bool vsync)
        {
            mVsync = vsync;
        }

        D3D12_VIEWPORT DX12Viewport::GetD3D12Viewport() const
        {
            return {
                .TopLeftX = 0,
                .TopLeftY = 0,
                .Width = static_cast<FLOAT>(mWidth),
                .Height = static_cast<FLOAT>(mHeight),
                .MinDepth = D3D12_MIN_DEPTH,
                .MaxDepth = D3D12_MAX_DEPTH
            };
        }

        void DX12Viewport::GetBackbuffers()
        {
            D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = mRTVDescriptorHeap.Get()->GetCPUDescriptorHandleForHeapStart();
            for (Uint32 i = 0; i < mBackbufferCount; ++i)
            {
                mSwapChain.Get()->GetBuffer(i, IID_PPV_ARGS(&mBackbuffers[i]));
                mDevice.GetDevice()->CreateRenderTargetView(mBackbuffers[i].Get(), nullptr, rtvHandle);
                mRTVDescriptors[i] = rtvHandle;
                rtvHandle.ptr += mRTVDescriptorSize;
            }
        }

        void DX12Viewport::ClearBackbuffers()
        {
            for (Uint32 i = 0; i < mBackbufferCount; ++i)
            {
                mBackbuffers[i] = nullptr;
                mRTVDescriptors[i].ptr = 0;
            }
        }
    } // namespace gapi
} // namespace cube
