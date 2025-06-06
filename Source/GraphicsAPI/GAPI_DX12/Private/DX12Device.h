#pragma once

#include "DX12Header.h"

#include "DX12CommandListManager.h"
#include "DX12DescriptorManager.h"
#include "DX12MemoryAllocator.h"
#include "DX12QueryManager.h"
#include "DX12QueueManager.h"
#include "DX12UploadManager.h"

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
        IDXGIAdapter3* GetAdapter3() const { return mAdapter3.Get(); }
        const DXGI_ADAPTER_DESC& GetAdapterDesc() const { return mAdapterDesc; }
        ID3D12Device* GetDevice() const { return mDevice.Get(); }

        DX12MemoryAllocator& GetMemoryAllocator() { return mMemoryAllocator; }
        DX12QueueManager& GetQueueManager() { return mQueueManager; }
        DX12UploadManager& GetUploadManager() { return mUploadManager; }
        DX12DescriptorManager& GetDescriptorManager() { return mDescriptorManager; }
        DX12CommandListManager& GetCommandListManager() { return mCommandListManager; }
        DX12QueryManager& GetQueryManager() { return mQueryManager; }

    private:
        ComPtr<IDXGIAdapter1> mAdapter;
        ComPtr<IDXGIAdapter3> mAdapter3;
        DXGI_ADAPTER_DESC mAdapterDesc;
        ComPtr<ID3D12Device> mDevice;

        DX12MemoryAllocator mMemoryAllocator;
        DX12QueueManager mQueueManager;
        DX12UploadManager mUploadManager;
        DX12DescriptorManager mDescriptorManager;
        DX12CommandListManager mCommandListManager;
        DX12QueryManager mQueryManager;

    };
} // namespace cube
