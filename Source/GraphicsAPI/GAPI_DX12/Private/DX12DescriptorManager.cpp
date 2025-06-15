#include "DX12DescriptorManager.h"

#include <numeric>

#include "DX12Device.h"
#include "DX12Utility.h"

namespace cube
{
    DX12DescriptorHandle DX12DescriptorHeap::AllocateCPU()
    {
        CHECK(mFreedIndicesCPU.size() > 0);

        Uint32 index = mFreedIndicesCPU.back();
        mFreedIndicesCPU.pop_back();

        return {
            .index = (int)index,
            .handle = {
                .ptr = mBeginCPU.ptr + (SIZE_T)mDescriptorSize * index
            }
        };
    }

    void DX12DescriptorHeap::FreeCPU(DX12DescriptorHandle descriptor)
    {
        CHECK_FORMAT(std::ranges::find(mFreedIndicesCPU, descriptor.index) == mFreedIndicesCPU.end(), "Freed the descriptor that already was freed.");

        mFreedIndicesCPU.push_back(descriptor.index);
    }

    D3D12_GPU_DESCRIPTOR_HANDLE DX12DescriptorHeap::AllocateGPU()
    {
        CHECK(mFreedIndicesGPU.size() > 0);

        Uint32 index = mFreedIndicesGPU.back();
        mFreedIndicesGPU.pop_back();

        return {
            .ptr = mBeginGPU.ptr + (SIZE_T)mDescriptorSize * index
        };
    }

    void DX12DescriptorHeap::FreeGPU(D3D12_GPU_DESCRIPTOR_HANDLE descriptor)
    {
        Uint32 index = (descriptor.ptr - mBeginGPU.ptr) / mDescriptorSize;
        CHECK_FORMAT(std::ranges::find(mFreedIndicesGPU, index) == mFreedIndicesGPU.end(), "Freed the descriptor that already was freed.");

        mFreedIndicesGPU.push_back(index);
    }

    void DX12DescriptorHeap::Initialize(DX12Device& device, const D3D12_DESCRIPTOR_HEAP_DESC& desc)
    {
        CHECK_HR(device.GetDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&mHeap)));

        mType = desc.Type;
        mNumDescriptors = desc.NumDescriptors;
        mDescriptorSize = device.GetDevice()->GetDescriptorHandleIncrementSize(desc.Type);

        mBeginCPU = mHeap->GetCPUDescriptorHandleForHeapStart();
        mFreedIndicesCPU.resize(desc.NumDescriptors);
        std::iota(mFreedIndicesCPU.begin(), mFreedIndicesCPU.end(), 0);
        std::reverse(mFreedIndicesCPU.begin(), mFreedIndicesCPU.end());
        mTotalNumIndicesCPU = static_cast<Uint32>(mFreedIndicesCPU.size());

        if ((desc.Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE) != 0)
        {
            mBeginGPU = mHeap->GetGPUDescriptorHandleForHeapStart();
            mFreedIndicesGPU.resize(desc.NumDescriptors);
            std::iota(mFreedIndicesGPU.begin(), mFreedIndicesGPU.end(), 0);
            mTotalNumIndicesGPU = static_cast<Uint32>(mFreedIndicesGPU.size());
        }
        else
        {
            mTotalNumIndicesGPU = 0;
        }
    }

    void DX12DescriptorHeap::Shutdown()
    {
        CHECK_FORMAT(mFreedIndicesCPU.size() == mTotalNumIndicesCPU, "All descriptors should be freed before shutdown descriptor heap.");
        CHECK_FORMAT(mFreedIndicesGPU.size() == mTotalNumIndicesGPU, "All descriptors should be freed before shutdown descriptor heap.");

        mHeap = nullptr;
    }

    DX12DescriptorManager::DX12DescriptorManager(DX12Device& device) :
        mDevice(device)
    {
    }

    void DX12DescriptorManager::Initialize()
    {
        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {
            .Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
            .NumDescriptors = 10,
            .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
            .NodeMask = 0
        };
        mRTVHeap.Initialize(mDevice, rtvHeapDesc);

        D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {
            .Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
            .NumDescriptors = 128,
            .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
            .NodeMask = 0
        };
        mSRVHeap.Initialize(mDevice, srvHeapDesc);

        D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {
            .Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
            .NumDescriptors = 8,
            .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
            .NodeMask = 0
        };
        mDSVHeap.Initialize(mDevice, dsvHeapDesc);

        D3D12_DESCRIPTOR_HEAP_DESC samplerHeapDesc = {
            .Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
            .NumDescriptors = 16,
            .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
            .NodeMask = 0
        };
        mSamplerHeap.Initialize(mDevice, samplerHeapDesc);

        mD3D12ShaderVislbleHeaps[0] = mSRVHeap.Get();
        mD3D12ShaderVislbleHeaps[1] = mSamplerHeap.Get();
    }

    void DX12DescriptorManager::Shutdown()
    {
        mSamplerHeap.Shutdown();
        mDSVHeap.Shutdown();
        mSRVHeap.Shutdown();
        mRTVHeap.Shutdown();
    }
} // namespace cube
