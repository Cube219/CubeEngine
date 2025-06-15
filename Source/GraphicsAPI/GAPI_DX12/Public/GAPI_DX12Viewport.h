#pragma once

#include "DX12Header.h"

#include "GAPI_Viewport.h"

#include "DX12APIObject.h"
#include "DX12DescriptorManager.h"
#include "DX12Fence.h"
#include "DX12MemoryAllocator.h"

#define MAX_BACKBUFFER_SIZE 5

namespace cube
{
    class DX12Device;

    namespace gapi
    {
        class CUBE_DX12_EXPORT DX12Viewport : public Viewport, public DX12APIObject
        {
        public:
            DX12Viewport(IDXGIFactory2* factory, DX12Device& device, const ViewportCreateInfo& createInfo);
            virtual ~DX12Viewport();

            DX12Viewport(const DX12Viewport& other) = delete;
            DX12Viewport& operator=(const DX12Viewport& rhs) = delete;

            virtual void AcquireNextImage() override;
            virtual void Present() override;

            virtual void Resize(Uint32 width, Uint32 height) override;
            virtual void SetVsync(bool vsync) override;

            ID3D12Resource* GetCurrentBackbuffer() const { return mBackbuffers[mCurrentIndex].Get(); }
            D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentRTVDescriptor() const { return mRTVDescriptors[mCurrentIndex]; }
            DX12DescriptorHandle GetDSVDescriptor() const { return mDSVDescriptor; }

            D3D12_VIEWPORT GetD3D12Viewport() const;

        private:
            void GetBackbuffers();
            void ClearBackbuffers();

            DX12Device& mDevice;

            ComPtr<IDXGISwapChain1> mSwapChain;
            ComPtr<IDXGISwapChain3> mSwapChain3;
            DXGI_FORMAT mSwapChainFormat;
            Array<ComPtr<ID3D12Resource>, MAX_BACKBUFFER_SIZE> mBackbuffers;
            ComPtr<ID3D12DescriptorHeap> mRTVDescriptorHeap;
            Uint32 mRTVDescriptorSize;
            Array<D3D12_CPU_DESCRIPTOR_HANDLE, MAX_BACKBUFFER_SIZE> mRTVDescriptors;

            Uint32 mWidth;
            Uint32 mHeight;
            Uint32 mBackbufferCount;
            bool mVsyncSupported;
            bool mVsync;

            Uint32 mCurrentIndex;
            Array<Uint64, MAX_BACKBUFFER_SIZE> mFenceValues;
            DX12Fence mFence;

            // TODO: Separate depth texture while implementing GAPI texture
            DX12Allocation mDepthBufferAllocation;
            DX12DescriptorHandle mDSVDescriptor;
        };
    } // namespace rapi
} // namespace cube
