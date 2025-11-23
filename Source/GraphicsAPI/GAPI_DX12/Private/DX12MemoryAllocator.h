#pragma once

#include "DX12Header.h"

#include "D3D12MemAlloc.h"

#include "DX12Utility.h"
#include "GAPI_Resource.h"

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
        gapi::ResourceUsage usage;
        void *pMapPtr = nullptr;

        D3D12MA::Allocation* allocation = nullptr;

        bool IsValid() const { return allocation != nullptr; }

        // (readBegin > readEnd) -> read all range
        void Map(Uint64 readBegin = 1, Uint64 readEnd = 0)
        {
            CHECK_FORMAT(usage != gapi::ResourceUsage::GPUOnly, "Cannot map the resouce used GPU only.");

            if (pMapPtr != nullptr)
            {
                return;
            }

            if (usage == gapi::ResourceUsage::CPUtoGPU)
            {
                readBegin = readEnd = 0; // Range [0, 0] -> cannot read, only write available
            }

            const D3D12_RANGE readRange = { readBegin, readEnd };
            CHECK_HR(allocation->GetResource()->Map(0, (readBegin > readEnd) ? nullptr : &readRange, &pMapPtr));
        }

        // (writeBegin > writeEnd) -> write all range
        void Unmap(Uint64 writeBegin = 1, Uint64 writeEnd = 0)
        {
            if (pMapPtr == nullptr)
            {
                return;
            }

            if (usage == gapi::ResourceUsage::GPUtoCPU)
            {
                writeBegin = writeEnd = 0; // Range [0, 0] -> Not be written
            }

            const D3D12_RANGE writtenRange = { writeBegin, writeEnd };
            allocation->GetResource()->Unmap(0, (writeBegin > writeEnd) ? nullptr : &writtenRange);

            pMapPtr = nullptr;
        }
    };

    class DX12MemoryAllocator
    {
    public:
        DX12MemoryAllocator(DX12Device& device);

        DX12MemoryAllocator(const DX12MemoryAllocator& other) = delete;
        DX12MemoryAllocator& operator=(const DX12MemoryAllocator& rhs) = delete;

        void Initialize();
        void Shutdown();

        DX12Allocation Allocate(D3D12_HEAP_TYPE heapType, const D3D12_RESOURCE_DESC& desc, const D3D12_CLEAR_VALUE* pOptimizedClearValue = nullptr);
        void Free(DX12Allocation& allocation);

    private:
        DX12Device& mDevice;

        ComPtr<D3D12MA::Allocator> mAllocator;
    };
} // namespace cube
