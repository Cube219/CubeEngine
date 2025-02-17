#include "DX12DescriptorManager.h"

#include "DX12Device.h"
#include "DX12Utility.h"

namespace cube
{
    D3D12_CPU_DESCRIPTOR_HANDLE DX12DescriptorHeap::AllocateCPU()
    {
        // TODO
        return {};
    }

    void DX12DescriptorHeap::FreeCPU(D3D12_CPU_DESCRIPTOR_HANDLE descriptor)
    {
    }

    D3D12_GPU_DESCRIPTOR_HANDLE DX12DescriptorHeap::AllocateGPU()
    {
        return {};
    }

    void DX12DescriptorHeap::FreeGPU(D3D12_GPU_DESCRIPTOR_HANDLE)
    {
    }

    void DX12DescriptorHeap::Initialize(DX12Device& device, const D3D12_DESCRIPTOR_HEAP_DESC& desc)
    {
        CHECK_HR(device.GetDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&mHeap)));

        mType = desc.Type;
        mNumDescriptors = desc.NumDescriptors;
        mDescriptorSize = device.GetDevice()->GetDescriptorHandleIncrementSize(desc.Type);
    }

    void DX12DescriptorHeap::Shutdown()
    {
        mHeap = nullptr;
    }

    DX12DescriptorManager::DX12DescriptorManager(DX12Device& device) :
        mDevice(device)
    {
    }

    void DX12DescriptorManager::Initialize()
    {
        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        rtvHeapDesc.NumDescriptors = 10;
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        mRTVHeap.Initialize(mDevice, rtvHeapDesc);

        D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
        srvHeapDesc.NumDescriptors = 64;
        srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        mSRVHeap.Initialize(mDevice, srvHeapDesc);
    }

    void DX12DescriptorManager::Shutdown()
    {
        mSRVHeap.Shutdown();
        mRTVHeap.Shutdown();
    }
} // namespace cube
