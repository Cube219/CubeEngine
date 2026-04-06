#include "DX12MemoryAllocator.h"

#include "Allocator/AllocatorUtility.h"
#include "DX12Device.h"
#include "DX12Utility.h"

namespace cube
{
    DX12MemoryAllocator::DX12MemoryAllocator(DX12Device& device)
        : mDevice(device)
        , mCurrentIndex(0)
    {
    }

    void DX12MemoryAllocator::Initialize(Uint32 numGPUSync)
    {
        D3D12MA::ALLOCATOR_DESC allocatorDesc = {};
        allocatorDesc.pDevice = mDevice.GetDevice();
        allocatorDesc.pAdapter = mDevice.GetAdapter();
        allocatorDesc.Flags = (D3D12MA::ALLOCATOR_FLAG_DEFAULT_POOLS_NOT_ZEROED | D3D12MA::ALLOCATOR_FLAG_MSAA_TEXTURES_ALWAYS_COMMITTED); // D3D12MA_RECOMMENDED_ALLOCATOR_FLAGS;

        CHECK_HR(D3D12MA::CreateAllocator(&allocatorDesc, &mAllocator));

        D3D12_HEAP_DESC transientHeapDesc = {};
        transientHeapDesc.SizeInBytes = DEFAULT_TRANSIENT_HEAP_SIZE;
        transientHeapDesc.Properties.Type = D3D12_HEAP_TYPE_DEFAULT;
        transientHeapDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
        // The device support resource heap tier 2. So create only one heap category.
        transientHeapDesc.Flags = D3D12_HEAP_FLAG_CREATE_NOT_ZEROED;
        mDevice.GetDevice()->CreateHeap(&transientHeapDesc, IID_PPV_ARGS(&mTransientHeap));
        SET_DEBUG_NAME(mTransientHeap, "Transient Heap");

        mCurrentTransientOffset = 0;
#if CUBE_DX12_TRACK_TRANSIENT_ALLOCATION
        // Start to 1. (0 is invalid)
        mCurrentTransientAllocationIndex = 1;
#endif

        SetNumGPUSync(numGPUSync);
    }

    void DX12MemoryAllocator::Shutdown()
    {
        mTransientHeap = nullptr;
        mAllocator = nullptr;
    }

    void DX12MemoryAllocator::SetNumGPUSync(Uint32 newNumGPUSync)
    {
    }

    void DX12MemoryAllocator::MoveToNextIndex(Uint64 nextGPUFrame)
    {
#if CUBE_DX12_TRACK_TRANSIENT_ALLOCATION
        CHECK_FORMAT(mCurrentAllocatedTransientIndices.empty(), "Not all transient allocations are freed before move to next frame!");
#endif

        // Aliasing barrier does not needed in different ExecuteCommandLists execution, so just reset the offset.
        mCurrentTransientOffset = 0;
    }

    DX12Allocation DX12MemoryAllocator::Allocate(D3D12_HEAP_TYPE heapType, const D3D12_RESOURCE_DESC& desc, bool transient, const D3D12_CLEAR_VALUE* pOptimizedClearValue)
    {
        DX12Allocation allocation;
        if (desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
        {
            allocation.type = DX12Allocation::ResourceType::Buffer;
        }
        else
        {
            allocation.type = DX12Allocation::ResourceType::Texture;
        }
        allocation.heapType = heapType;
        allocation.isTransient = transient;

        if (transient)
        {
            if (heapType != D3D12_HEAP_TYPE_DEFAULT)
            {
                CUBE_LOG(Warning, DX12MemoryAllocator, "Transient resource must use default heap type. Force using default heap type.");
                allocation.heapType = D3D12_HEAP_TYPE_DEFAULT;
            }

            D3D12_RESOURCE_DESC newDesc = desc;
            if (mDevice.IsTightAlignmentSupported())
            {
                newDesc.Flags |= D3D12_RESOURCE_FLAG_USE_TIGHT_ALIGNMENT;
            }
            const D3D12_RESOURCE_ALLOCATION_INFO allocationInfo = mDevice.GetDevice()->GetResourceAllocationInfo(0, 1, &newDesc);

            // TODO: Handle out of memory.
            const Uint64 offset = Align(mCurrentTransientOffset, allocationInfo.Alignment);
            CHECK_HR(mDevice.GetDevice()->CreatePlacedResource(mTransientHeap.Get(), offset, &newDesc, D3D12_RESOURCE_STATE_COMMON, pOptimizedClearValue, IID_PPV_ARGS(&allocation.resource)));
            mCurrentTransientOffset = offset + allocationInfo.SizeInBytes;
#if CUBE_DX12_TRACK_TRANSIENT_ALLOCATION
            allocation.transientAllocationIndex = mCurrentTransientAllocationIndex;
            mCurrentAllocatedTransientIndices.insert(mCurrentTransientAllocationIndex);
            mCurrentTransientAllocationIndex++;
#endif
        }
        else
        {
            D3D12MA::ALLOCATION_DESC allocationDesc = {};
            allocationDesc.HeapType = heapType;

            D3D12_RESOURCE_STATES states = D3D12_RESOURCE_STATE_COMMON;
            if (heapType == D3D12_HEAP_TYPE_UPLOAD)
            {
                states = D3D12_RESOURCE_STATE_GENERIC_READ;
            }
            CHECK_HR(mAllocator->CreateResource(&allocationDesc, &desc, states, pOptimizedClearValue, &allocation.allocation, IID_NULL, nullptr));
            allocation.resource = allocation.allocation->GetResource();
        }

        return allocation;
    }

    void DX12MemoryAllocator::Free(DX12Allocation& allocation)
    {
        allocation.Unmap();

        if (allocation.isTransient)
        {
            allocation.resource->Release();
#if CUBE_DX12_TRACK_TRANSIENT_ALLOCATION
            mCurrentAllocatedTransientIndices.erase(allocation.transientAllocationIndex);
            allocation.transientAllocationIndex = 0;
#endif
        }

        if (allocation.allocation)
        {
            allocation.allocation->Release();
            allocation.allocation = nullptr;
        }
    }
} // namespace cube
