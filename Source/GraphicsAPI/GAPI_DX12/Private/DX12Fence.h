#pragma once

#include "DX12Header.h"

namespace cube
{
    class DX12Device;

    class DX12Fence
    {
    public:
        DX12Fence(DX12Device& device);
        ~DX12Fence();

        void Initialize();
        void Shutdown();

        void Signal(ID3D12CommandQueue* queue, Uint64 fenceValue);
        void Wait(Uint64 fenceValue);

    private:
        DX12Device& mDevice;

        ComPtr<ID3D12Fence> mFence;
        HANDLE mFenceEvent;
        Uint64 mLastSignalFenceValue;
    };
} // namespace cube
