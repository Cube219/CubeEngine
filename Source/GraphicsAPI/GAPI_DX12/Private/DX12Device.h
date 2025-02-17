#pragma once

#include "DX12Header.h"

#include "DX12CommandListManager.h"
#include "DX12DescriptorManager.h"
#include "DX12QueueManager.h"

namespace cube
{
    class DX12Device
    {
    public:
        DX12Device();
        ~DX12Device();

        DX12Device(const DX12Device& other) = delete;
        DX12Device& operator=(const DX12Device& rhs) = delete;

        void Initialize(const ComPtr<IDXGIAdapter1>& adapter);
        void Shutdown();

        IDXGIAdapter1* GetAdapter() const { return mAdapter.Get(); }
        const DXGI_ADAPTER_DESC& GetAdapterDesc() const { return mAdapterDesc; }
        ID3D12Device* GetDevice() const { return mDevice.Get(); }

        DX12QueueManager& GetQueueManager() { return mQueueManager; }
        DX12DescriptorManager& GetDescriptorManager() { return mDescriptorManager; }
        DX12CommandListManager& GetCommandListManager() { return mCommandListManager; }

    private:
        ComPtr<IDXGIAdapter1> mAdapter;
        DXGI_ADAPTER_DESC mAdapterDesc;
        ComPtr<ID3D12Device> mDevice;

        DX12QueueManager mQueueManager;
        DX12DescriptorManager mDescriptorManager;
        DX12CommandListManager mCommandListManager;

    };
} // namespace cube
