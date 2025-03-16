#include "DX12MemoryAllocator.h"

#include "DX12Device.h"
#include "DX12Utility.h"

namespace cube
{
    DX12MemoryAllocator::DX12MemoryAllocator(DX12Device& device) :
        mDevice(device)
    {
    }

    void DX12MemoryAllocator::Initialize()
    {
        D3D12MA::ALLOCATOR_DESC allocatorDesc = {};
        allocatorDesc.pDevice = mDevice.GetDevice();
        allocatorDesc.pAdapter = mDevice.GetAdapter();
        allocatorDesc.Flags = (D3D12MA::ALLOCATOR_FLAG_DEFAULT_POOLS_NOT_ZEROED | D3D12MA::ALLOCATOR_FLAG_MSAA_TEXTURES_ALWAYS_COMMITTED); // D3D12MA_RECOMMENDED_ALLOCATOR_FLAGS;

        CHECK_HR(D3D12MA::CreateAllocator(&allocatorDesc, &mAllocator));
    }

    void DX12MemoryAllocator::Shutdown()
    {
        mAllocator = nullptr;
    }

    DX12Allocation DX12MemoryAllocator::Allocate(D3D12_HEAP_TYPE heapType, const D3D12_RESOURCE_DESC& desc)
    {
        DX12Allocation allocation;

        D3D12MA::ALLOCATION_DESC allocationDesc = {};
        allocationDesc.HeapType = heapType;

        D3D12_RESOURCE_STATES states = D3D12_RESOURCE_STATE_COMMON;
        if (heapType == D3D12_HEAP_TYPE_UPLOAD)
        {
            states = D3D12_RESOURCE_STATE_GENERIC_READ;
        }
        CHECK_HR(mAllocator->CreateResource(&allocationDesc, &desc, states, nullptr, &allocation.allocation, IID_NULL, nullptr));

        return allocation;
    }

    void DX12MemoryAllocator::Free(DX12Allocation& allocation)
    {
        allocation.Unmap();

        allocation.allocation->Release();
        allocation.allocation = nullptr;
    }
} // namespace cube
