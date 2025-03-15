#pragma once

#include "DX12Header.h"

#include "D3D12MemAlloc.h"

namespace cube
{
    class DX12Device;

    class DX12MemoryAllocator
    {
    public:
        DX12MemoryAllocator(DX12Device& device);

        DX12MemoryAllocator(const DX12MemoryAllocator& other) = delete;
        DX12MemoryAllocator& operator=(const DX12MemoryAllocator& rhs) = delete;

        void Initialize();
        void Shutdown();

    private:
        DX12Device& mDevice;

        ComPtr<D3D12MA::Allocator> mAllocator;
    };
} // namespace cube
