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
                CUBE_LOG(Warning, DX12, "Cannot cast IDXGISwapChain3. Frame buffering cannot be used. (HR: {0})", res);
            }

            // Backbuffer / RTV
            D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
            rtvHeapDesc.NumDescriptors = mBackbufferCount;
            rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
            CHECK_HR(device.GetDevice()->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&mRTVDescriptorHeap)));
            mRTVDescriptorSize = device.GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

            GetBackbuffers();

            mFence.Initialize(CUBE_T("ViewportFence"));
        }

        DX12Viewport::~DX12Viewport()
        {
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

            D3D12_RESOURCE_DESC desc = {
                .Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
                .Alignment = 0,
                .Width = mWidth,
                .Height = mHeight,
                .DepthOrArraySize = 1,
                .MipLevels = 1,
                .Format = DXGI_FORMAT_D32_FLOAT,
                .SampleDesc = {
                    .Count = 1,
                    .Quality = 0 },
                .Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
                .Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
            };
            D3D12_CLEAR_VALUE clearValue = {
                .Format = DXGI_FORMAT_D32_FLOAT,
                .DepthStencil = {
                    .Depth = 0.0f,
                    .Stencil = 0
                }
            };
            mDepthBufferAllocation = mDevice.GetMemoryAllocator().Allocate(D3D12_HEAP_TYPE_DEFAULT, desc, &clearValue);
            D3D12_DEPTH_STENCIL_VIEW_DESC viewDesc = {};
            viewDesc.Format = DXGI_FORMAT_D32_FLOAT;
            viewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
            viewDesc.Flags = D3D12_DSV_FLAG_NONE;
            mDSVDescriptor = mDevice.GetDescriptorManager().GetDSVHeap().AllocateCPU();
            mDevice.GetDevice()->CreateDepthStencilView(mDepthBufferAllocation.allocation->GetResource(), &viewDesc, mDSVDescriptor.handle);

            ComPtr<ID3D12GraphicsCommandList> cmdList;
            mDevice.GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mDevice.GetCommandListManager().GetCurrentAllocator(), nullptr, IID_PPV_ARGS(&cmdList));
            D3D12_RESOURCE_BARRIER barrier = {
                .Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
                .Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
                .Transition = {
                    .pResource = mDepthBufferAllocation.allocation->GetResource(),
                    .Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
                    .StateBefore = D3D12_RESOURCE_STATE_COMMON,
                    .StateAfter = D3D12_RESOURCE_STATE_DEPTH_WRITE }
            };
            cmdList->ResourceBarrier(1, &barrier);

            cmdList->Close();
            ID3D12CommandList* cmdLists[] = { cmdList.Get() };
            mDevice.GetQueueManager().GetMainQueue()->ExecuteCommandLists(1, cmdLists);
            mDevice.GetCommandListManager().MoveToNextAllocator(); // NOTE: temporal code. Use more good way? (Use separate command list pool for stage/upload?)
        }

        void DX12Viewport::ClearBackbuffers()
        {
            for (Uint32 i = 0; i < mBackbufferCount; ++i)
            {
                mBackbuffers[i] = nullptr;
                mRTVDescriptors[i].ptr = 0;
            }

            mDevice.GetDescriptorManager().GetDSVHeap().FreeCPU(mDSVDescriptor);
            mDevice.GetMemoryAllocator().Free(mDepthBufferAllocation);
        }
    } // namespace gapi
} // namespace cube
