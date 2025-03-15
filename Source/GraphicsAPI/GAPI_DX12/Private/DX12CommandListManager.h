#pragma once

#include "DX12Header.h"

#include "DX12Fence.h"

#define MAX_ALLOCATOR_SIZE 5

namespace cube
{
    class DX12Device;

    class DX12CommandListManager
    {
    public:
        DX12CommandListManager(DX12Device& device);

        DX12CommandListManager(const DX12CommandListManager& other) = delete;
        DX12CommandListManager& operator=(const DX12CommandListManager& rhs) = delete;

        void Initialize();
        void Shutdown();

        void WaitCurrentAllocatorIsReady();

        void Reset();
        ID3D12CommandAllocator* GetAllocator();

        void MoveToNextAllocator();

    private:
        DX12Device& mDevice;

        Array<ComPtr<ID3D12CommandAllocator>, MAX_ALLOCATOR_SIZE> mAllocators;
        Uint32 mCurrentIndex;
        DX12Fence mFence;
        Array<Uint64, MAX_ALLOCATOR_SIZE> mFenceValues;
    };
} // namespace cube
