#pragma once

#include "DX12Header.h"

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
        void *pMapPtr = nullptr;

        D3D12MA::Allocation* allocation;

        void Map()
        {
            if (pMapPtr != nullptr)
            {
                return;
            }

            D3D12_RANGE range = { 0, 0 };
            CHECK_HR(allocation->GetResource()->Map(0, &range, &pMapPtr));
        }

        void Unmap()
        {
            if (pMapPtr == nullptr)
            {
                return;
            }

            allocation->GetResource()->Unmap(0, NULL);
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
