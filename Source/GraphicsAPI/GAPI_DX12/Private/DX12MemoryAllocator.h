#pragma once

#include "DX12Header.h"

#define D3D12MA_D3D12_HEADERS_ALREADY_INCLUDED
#include "D3D12MemAlloc.h"

#include "DX12Utility.h"

namespace cube
{
    class DX12Device;

    struct DX12Allocation
    {
        enum class ResourceType
        {
            Buffer, Texture
        };
        ResourceType type;
        D3D12_HEAP_TYPE heapType;
        bool isTransient;
        void *pMapPtr = nullptr;

        D3D12MA::Allocation* allocation = nullptr;
        ID3D12Resource* resource = nullptr;

        bool IsValid() const { return resource != nullptr; }

        // (readBegin > readEnd) -> read all range
        void Map(Uint64 readBegin = 1, Uint64 readEnd = 0)
        {
            CHECK_FORMAT(heapType != D3D12_HEAP_TYPE_DEFAULT, "Cannot map the resource from default heap type.");

            if (pMapPtr != nullptr)
            {
                return;
            }

            if (heapType == D3D12_HEAP_TYPE_UPLOAD || heapType == D3D12_HEAP_TYPE_GPU_UPLOAD)
            {
                readBegin = readEnd = 0; // Range [0, 0] -> cannot read, only write available
            }

            const D3D12_RANGE readRange = { readBegin, readEnd };
            CHECK_HR(resource->Map(0, (readBegin > readEnd) ? nullptr : &readRange, &pMapPtr));
        }

        // (writeBegin > writeEnd) -> write all range
        void Unmap(Uint64 writeBegin = 1, Uint64 writeEnd = 0)
        {
            if (pMapPtr == nullptr)
            {
                return;
            }

            if (heapType == D3D12_HEAP_TYPE_READBACK)
            {
                writeBegin = writeEnd = 0; // Range [0, 0] -> Not be written
            }

            const D3D12_RANGE writtenRange = { writeBegin, writeEnd };
            resource->Unmap(0, (writeBegin > writeEnd) ? nullptr : &writtenRange);

            pMapPtr = nullptr;
        }
    };

    class DX12MemoryAllocator
    {
    public:
        static constexpr Uint64 DEFAULT_TRANSIENT_HEAP_SIZE = 128u * 1024 * 1024; // 128 MiB

    public:
        DX12MemoryAllocator(DX12Device& device);

        DX12MemoryAllocator(const DX12MemoryAllocator& other) = delete;
        DX12MemoryAllocator& operator=(const DX12MemoryAllocator& rhs) = delete;

        void Initialize(Uint32 numGPUSync);
        void Shutdown();

        void SetNumGPUSync(Uint32 newNumGPUSync);
        void MoveToNextIndex(Uint64 nextGPUFrame);

        DX12Allocation Allocate(D3D12_HEAP_TYPE heapType, const D3D12_RESOURCE_DESC& desc, bool transient = false, const D3D12_CLEAR_VALUE* pOptimizedClearValue = nullptr);
        void Free(DX12Allocation& allocation);

    private:
        void AllocateFromTransient(DX12Allocation& inOutAllocation, const D3D12_RESOURCE_DESC& desc, const D3D12_CLEAR_VALUE* pOptimizedClearValue = nullptr);

        DX12Device& mDevice;

        Uint32 mNumGPUSync;
        Uint64 mCurrentGPUFrame;

        ComPtr<D3D12MA::Allocator> mAllocator;

        struct TransientHeap
        {
            ComPtr<ID3D12Heap> d3d12Heap;
            Uint64 size;
            Uint64 currentOffset;
            Uint64 lastUsedGPUFrame;
        };
        TransientHeap* CreateNewTransientHeap(Uint64 size);
        void ClearUnusedTransientHeaps(Uint64 gpuFrame);

        Vector<TransientHeap> mTransientHeaps;
    };
} // namespace cube
