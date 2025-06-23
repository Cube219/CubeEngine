#pragma once

#include "DX12Header.h"

namespace cube
{
    class DX12Device;

    using DX12FenceValue = Uint64;

    class DX12Fence
    {
    public:
        DX12Fence(DX12Device& device);
        ~DX12Fence();

        void Initialize(StringView debugName);
        void Shutdown(bool skipPendingSignal = false);

        void Signal(ID3D12CommandQueue* queue, Uint64 fenceValue);
        void Wait(DX12FenceValue fenceValue);
        DX12FenceValue GetCompletedValue();

    private:
        DX12Device& mDevice;

        ComPtr<ID3D12Fence> mFence;
        HANDLE mFenceEvent;
        DX12FenceValue mLastSignalFenceValue;
    };
} // namespace cube
