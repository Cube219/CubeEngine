#pragma once

#include "DX12Header.h"

namespace cube
{
    class DX12Device;

    class DX12DescriptorHeap
    {
    public:
        ID3D12DescriptorHeap* Get() const { return mHeap.Get(); }

        D3D12_CPU_DESCRIPTOR_HANDLE AllocateCPU();
        void FreeCPU(D3D12_CPU_DESCRIPTOR_HANDLE descriptor);

        D3D12_GPU_DESCRIPTOR_HANDLE AllocateGPU();
        void FreeGPU(D3D12_GPU_DESCRIPTOR_HANDLE descriptor);

    private:
        friend class DX12DescriptorManager;

        DX12DescriptorHeap() = default;

        void Initialize(DX12Device& device, const D3D12_DESCRIPTOR_HEAP_DESC& desc);
        void Shutdown();

        ComPtr<ID3D12DescriptorHeap> mHeap;
        D3D12_DESCRIPTOR_HEAP_TYPE mType;
        Uint32 mNumDescriptors;
        Uint32 mDescriptorSize;

        D3D12_CPU_DESCRIPTOR_HANDLE mBeginCPU;
        D3D12_GPU_DESCRIPTOR_HANDLE mBeginGPU;

        Vector<Uint32> mFreedIndicesCPU;
        Uint32 mTotalNumIndicesCPU;
        Vector<Uint32> mFreedIndicesGPU;
        Uint32 mTotalNumIndicesGPU;
        // NOTE: when try to increase descriptor heap size, you should re-init imgui in GAPI_DX12::InitializeImGUI()
        // because it uses raw SRV heap
    };

    class DX12DescriptorManager
    {
    public:
        DX12DescriptorManager(DX12Device& device);

        DX12DescriptorManager(const DX12DescriptorManager& other) = delete;
        DX12DescriptorManager& operator=(const DX12DescriptorManager& rhs) = delete;

        void Initialize();
        void Shutdown();

        DX12DescriptorHeap& GetRTVHeap() { return mRTVHeap; }
        DX12DescriptorHeap& GetSRVHeap() { return mSRVHeap; }
        DX12DescriptorHeap& GetDSVHeap() { return mDSVHeap; }

    private:
        DX12Device& mDevice;

        DX12DescriptorHeap mRTVHeap;
        DX12DescriptorHeap mSRVHeap;
        DX12DescriptorHeap mDSVHeap;
    };
} // namespace cube
