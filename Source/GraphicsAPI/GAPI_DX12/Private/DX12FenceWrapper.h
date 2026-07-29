#pragma once

#include "DX12Header.h"

namespace cube
{
    class DX12Device;

    class DX12FenceWrapper
    {
    public:
        DX12FenceWrapper(DX12Device& device);
        ~DX12FenceWrapper();

        void Initialize(StringView debugName, bool allowCPUAccess);
        void Shutdown(bool skipPendingSignal = false);

        void SignalToQueue(ID3D12CommandQueue* queue, Uint64 fenceValue);
        void WaitOnQueue(ID3D12CommandQueue* queue, Uint64 fenceValue);
        
        void Signal(Uint64 fenceValue);
        void Wait(Uint64 fenceValue);

        Uint64 GetCompletedValue();

    private:
        DX12Device& mDevice;

        ComPtr<ID3D12Fence> mFence;
        HANDLE mFenceEvent;
        Uint64 mLastSignalFenceValue;
    };
} // namespace cube
