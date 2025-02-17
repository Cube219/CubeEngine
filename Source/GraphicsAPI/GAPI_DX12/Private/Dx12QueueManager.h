#pragma once

#include "DX12Header.h"

namespace cube
{
    class DX12Device;

    class DX12QueueManager
    {
    public:
        DX12QueueManager(DX12Device& device);

        DX12QueueManager(const DX12QueueManager& other) = delete;
        DX12QueueManager& operator=(const DX12QueueManager& rhs) = delete;

        void Initialize();
        void Shutdown();

        ID3D12CommandQueue* GetMainQueue() const { return mMainQueue.Get(); }

    private:
        DX12Device& mDevice;

        ComPtr<ID3D12CommandQueue> mMainQueue;
    };
} // namespace cube
