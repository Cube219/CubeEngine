#include "DX12DescriptorManager.h"

#include <numeric>

#include "DX12Device.h"
#include "DX12Utility.h"

namespace cube
{
    DX12DescriptorHandle DX12DescriptorHeap::Allocate()
    {
        CHECK(mFreedIndices.size() > 0);

        Uint32 index = mFreedIndices.back();
        mFreedIndices.pop_back();

        return {
            .index = (int)index,
            .cpuHandle = {
                .ptr = mBeginCPU.ptr + (SIZE_T)mDescriptorSize * index
            },
            .gpuHandle = {
                .ptr = mBeginGPU.ptr ? mBeginGPU.ptr + (SIZE_T)mDescriptorSize * index : 0
            }
        };
    }

    void DX12DescriptorHeap::Free(DX12DescriptorHandle& descriptor)
    {
        CHECK_FORMAT(descriptor.index >= 0, "Try to free invalid descriptor.");
        CHECK_FORMAT(std::ranges::find(mFreedIndices, descriptor.index) == mFreedIndices.end(), "Freed the descriptor that already was freed.");

        mFreedIndices.push_back(descriptor.index);
        descriptor.index = -1;
    }

    void DX12DescriptorHeap::Free(D3D12_CPU_DESCRIPTOR_HANDLE descriptor)
    {
        Uint32 index = (descriptor.ptr - mBeginCPU.ptr) / mDescriptorSize;
        CHECK_FORMAT(std::ranges::find(mFreedIndices, index) == mFreedIndices.end(), "Freed the descriptor that already was freed.");

        mFreedIndices.push_back(index);
    }

    void DX12DescriptorHeap::Initialize(DX12Device& device, const D3D12_DESCRIPTOR_HEAP_DESC& desc, StringView debugName)
    {
        CHECK_HR(device.GetDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&mHeap)));
        SET_DEBUG_NAME(mHeap, debugName);

        mType = desc.Type;
        // TODO: Check possible number of descriptors.
        mNumDescriptors = desc.NumDescriptors;
        mDescriptorSize = device.GetDevice()->GetDescriptorHandleIncrementSize(desc.Type);

        mBeginCPU = mHeap->GetCPUDescriptorHandleForHeapStart();
        mFreedIndices.resize(desc.NumDescriptors);
        std::iota(mFreedIndices.begin(), mFreedIndices.end(), 0);
        std::reverse(mFreedIndices.begin(), mFreedIndices.end());
        mTotalNumIndices = static_cast<Uint32>(mFreedIndices.size());

        if ((desc.Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE) != 0)
        {
            mBeginGPU = mHeap->GetGPUDescriptorHandleForHeapStart();
        }
        else
        {
            mBeginGPU.ptr = 0;
        }
    }

    void DX12DescriptorHeap::Shutdown()
    {
        CHECK_FORMAT(mFreedIndices.size() == mTotalNumIndices, "All descriptors should be freed before shutdown descriptor heap.");

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
        mRTVHeap.Initialize(mDevice, rtvHeapDesc, CUBE_T("DescriptorHeap(RTV)"));

        D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {
            .Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
            .NumDescriptors = 1024,
            .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
            .NodeMask = 0
        };
        mSRVHeap.Initialize(mDevice, srvHeapDesc, CUBE_T("DescriptorHeap(CBV_SRV_UAV)"));

        D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {
            .Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
            .NumDescriptors = 8,
            .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
            .NodeMask = 0
        };
        mDSVHeap.Initialize(mDevice, dsvHeapDesc, CUBE_T("DescriptorHeap(DSV)"));

        D3D12_DESCRIPTOR_HEAP_DESC samplerHeapDesc = {
            .Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
            .NumDescriptors = 64,
            .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
            .NodeMask = 0
        };
        mSamplerHeap.Initialize(mDevice, samplerHeapDesc, CUBE_T("DescriptorHeap(Sampler)"));

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
