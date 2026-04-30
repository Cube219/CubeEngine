#include "DX12MemoryAllocator.h"

#include "Allocator/AllocatorUtility.h"
#include "DX12Device.h"
#include "DX12Utility.h"

namespace cube
{
    DX12MemoryAllocator::DX12MemoryAllocator(DX12Device& device)
        : mDevice(device)
    {
    }

    void DX12MemoryAllocator::Initialize(Uint32 numGPUSync)
    {
        D3D12MA::ALLOCATOR_DESC allocatorDesc = {};
        allocatorDesc.pDevice = mDevice.GetDevice();
        allocatorDesc.pAdapter = mDevice.GetAdapter();
        allocatorDesc.Flags = (D3D12MA::ALLOCATOR_FLAG_DEFAULT_POOLS_NOT_ZEROED | D3D12MA::ALLOCATOR_FLAG_MSAA_TEXTURES_ALWAYS_COMMITTED); // D3D12MA_RECOMMENDED_ALLOCATOR_FLAGS;

        CHECK_HR(D3D12MA::CreateAllocator(&allocatorDesc, &mAllocator));

        SetNumGPUSync(numGPUSync);
        mCurrentGPUFrame = 0;

        // Allocate initial transient heap.
        CreateNewTransientHeap(0);
    }

    void DX12MemoryAllocator::Shutdown()
    {
        mTransientHeaps.clear();
        mAllocator = nullptr;
    }

    void DX12MemoryAllocator::SetNumGPUSync(Uint32 newNumGPUSync)
    {
        mNumGPUSync = newNumGPUSync;
    }

    void DX12MemoryAllocator::MoveToNextIndex(Uint64 nextGPUFrame)
    {
        mCurrentGPUFrame = nextGPUFrame;
        ClearUnusedTransientHeaps(mCurrentGPUFrame);

        // Aliasing barrier is not needed in different ExecuteCommandLists execution, so just reset the offset.
        for (TransientHeap& heap : mTransientHeaps)
        {
            heap.currentOffset = 0;
        }
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

        D3D12_RESOURCE_DESC newDesc = desc;
        if (mDevice.IsTightAlignmentSupported())
        {
            newDesc.Alignment = 0;
            newDesc.Flags |= D3D12_RESOURCE_FLAG_USE_TIGHT_ALIGNMENT;
        }

        if (transient)
        {
            if (heapType != D3D12_HEAP_TYPE_DEFAULT)
            {
                CUBE_LOG(Warning, DX12MemoryAllocator, "Transient resource must use default heap type. Force using default heap type.");
                allocation.heapType = D3D12_HEAP_TYPE_DEFAULT;
            }

            AllocateFromTransient(allocation, newDesc, pOptimizedClearValue);
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
            else if (heapType == D3D12_HEAP_TYPE_READBACK)
            {
                states = D3D12_RESOURCE_STATE_COPY_DEST;
            }
            CHECK_HR(mAllocator->CreateResource(&allocationDesc, &newDesc, states, pOptimizedClearValue, &allocation.allocation, IID_NULL, nullptr));
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
        }

        if (allocation.allocation)
        {
            allocation.allocation->Release();
            allocation.allocation = nullptr;
        }
        allocation.resource = nullptr;
    }

    void DX12MemoryAllocator::AllocateFromTransient(DX12Allocation& inOutAllocation, const D3D12_RESOURCE_DESC& desc, const D3D12_CLEAR_VALUE* pOptimizedClearValue)
    {
        const D3D12_RESOURCE_ALLOCATION_INFO allocationInfo = mDevice.GetDevice()->GetResourceAllocationInfo(0, 1, &desc);

        TransientHeap* selectedHeap = nullptr;
        Uint64 alignedOffset = 0;
        for (TransientHeap& heap : mTransientHeaps)
        {
            alignedOffset = Align(heap.currentOffset, allocationInfo.Alignment);
            if (alignedOffset + allocationInfo.SizeInBytes <= heap.size)
            {
                selectedHeap = &heap;
                break;
            }
        }
        if (!selectedHeap)
        {
            selectedHeap = CreateNewTransientHeap(allocationInfo.SizeInBytes);
        }

        CHECK_HR(mDevice.GetDevice()->CreatePlacedResource(selectedHeap->d3d12Heap.Get(), alignedOffset, &desc, D3D12_RESOURCE_STATE_COMMON, pOptimizedClearValue, IID_PPV_ARGS(&inOutAllocation.resource)));
        selectedHeap->currentOffset = alignedOffset + allocationInfo.SizeInBytes;
        selectedHeap->lastUsedGPUFrame = mCurrentGPUFrame;
    }

    DX12MemoryAllocator::TransientHeap* DX12MemoryAllocator::CreateNewTransientHeap(Uint64 size)
    {
        const Uint64 heapAlignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
        const Uint64 heapSize = std::max(Align(size, heapAlignment), DEFAULT_TRANSIENT_HEAP_SIZE);

        D3D12_HEAP_DESC transientHeapDesc = {};
        transientHeapDesc.SizeInBytes = heapSize;
        transientHeapDesc.Properties.Type = D3D12_HEAP_TYPE_DEFAULT;
        transientHeapDesc.Alignment = heapAlignment;
        // The device supports resource heap tier 2. So create only one heap category.
        transientHeapDesc.Flags = mDevice.IsNonZeroInHeapCreationSupported() ? D3D12_HEAP_FLAG_CREATE_NOT_ZEROED : D3D12_HEAP_FLAG_NONE;

        ComPtr<ID3D12Heap> newHeap;
        CHECK_HR(mDevice.GetDevice()->CreateHeap(&transientHeapDesc, IID_PPV_ARGS(&newHeap)));
        SET_DEBUG_NAME(newHeap, "Transient Heap");

        mTransientHeaps.push_back({
            .d3d12Heap = newHeap,
            .size = heapSize,
            .currentOffset = 0,
            .lastUsedGPUFrame = mCurrentGPUFrame
        });
        return &mTransientHeaps.back();
    }

    void DX12MemoryAllocator::ClearUnusedTransientHeaps(Uint64 gpuFrame)
    {
        if (gpuFrame < mNumGPUSync)
        {
            return;
        }

        for (int i = static_cast<int>(mTransientHeaps.size()) - 1; i >= 0; --i)
        {
            if (mTransientHeaps[i].lastUsedGPUFrame <= gpuFrame - mNumGPUSync)
            {
                const int lastIndex = static_cast<int>(mTransientHeaps.size()) - 1;
                if (lastIndex > i)
                {
                    mTransientHeaps[i] = std::move(mTransientHeaps[lastIndex]);
                }
                mTransientHeaps.pop_back();
            }
        }
    }
} // namespace cube
