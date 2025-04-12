#include "DX12Device.h"

#include "DX12Utility.h"

namespace cube
{
    DX12Device::DX12Device() :
        mMemoryAllocator(*this),
        mQueueManager(*this),
        mDescriptorManager(*this),
        mCommandListManager(*this),
        mQueryManager(*this)
    {
    }

    DX12Device::~DX12Device()
    {
    }

    void DX12Device::Initialize(const ComPtr<IDXGIAdapter1>& adapter)
    {
        mAdapter = adapter;

		CHECK_HR(mAdapter->GetDesc(&mAdapterDesc));

        HRESULT res = D3D12CreateDevice(mAdapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&mDevice));
        CHECK_HR(res);

        mMemoryAllocator.Initialize();
        mQueueManager.Initialize();
        mDescriptorManager.Initialize();
        mCommandListManager.Initialize();
        mQueryManager.Initialize();
    }

    void DX12Device::Shutdown()
    {
        mQueryManager.Shutdown();
        mCommandListManager.Shutdown();
        mDescriptorManager.Shutdown();
        mQueueManager.Shutdown();
        mMemoryAllocator.Shutdown();

        mDevice = nullptr;
        mAdapter = nullptr;
    }
} // namespace cube
