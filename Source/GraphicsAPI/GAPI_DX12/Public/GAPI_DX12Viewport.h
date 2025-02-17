#pragma once

#include "DX12Header.h"

#include "GAPI_Viewport.h"

#include "DX12Fence.h"

#define MAX_BACKBUFFER_SIZE 5

namespace cube
{
    class DX12Device;

	namespace gapi
	{
        class CUBE_DX12_EXPORT DX12Viewport : public Viewport
	    {
	    public:
            DX12Viewport(IDXGIFactory2* factory, DX12Device& device, const ViewportCreateInfo& createInfo);
			virtual ~DX12Viewport();

			virtual void AcquireNextImage() override;
			virtual void Present() override;

			virtual void Resize(Uint32 width, Uint32 height) override;
			virtual void SetVsync(bool vsync) override;

			ID3D12Resource* GetCurrentBackbuffer() const { return mBackbuffers[mCurrentIndex].Get(); }
			D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentRTVDescriptor() const { return mRTVDescriptors[mCurrentIndex]; }

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

			char mPadding[24]; // VS bug? (Try to use ASAN)
        };
	} // namespace rapi
} // namespace cube
