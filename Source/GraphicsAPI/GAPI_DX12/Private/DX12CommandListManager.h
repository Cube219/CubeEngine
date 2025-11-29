#pragma once

#include "DX12Header.h"

#include "DX12Fence.h"

namespace cube
{
    class DX12APIObject;
    class DX12Device;

    class DX12CommandListManager
    {
    public:
        DX12CommandListManager(DX12Device& device);

        DX12CommandListManager(const DX12CommandListManager& other) = delete;
        DX12CommandListManager& operator=(const DX12CommandListManager& rhs) = delete;

        void Initialize(Uint32 numGPUSync);
        void Shutdown();

        void SetNumGPUSync(Uint32 newNumGPUSync);
        void MoveToNextIndex(Uint64 nextGPUFrame);

        ID3D12CommandAllocator* GetCurrentAllocator() const { return mAllocators[mCurrentIndex].Get(); }


    private:
        DX12Device& mDevice;

        Uint32 mCurrentIndex;

        Vector<ComPtr<ID3D12CommandAllocator>> mAllocators;
    };
} // namespace cube
