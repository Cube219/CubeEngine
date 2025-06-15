#include "DX12Device.h"

#include "DX12Utility.h"
#include "Windows/WindowsString.h"

namespace cube
{
    DX12Device::DX12Device() :
        mMemoryAllocator(*this),
        mQueueManager(*this),
        mUploadManager(*this),
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
        mAdapter.As(&mAdapter3);

		CHECK_HR(mAdapter->GetDesc(&mAdapterDesc));

        HRESULT res = D3D12CreateDevice(mAdapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&mDevice));
        CHECK_HR(res);

        CHECK_HR(mFeatureSupport.Init(mDevice.Get()));

        mMemoryAllocator.Initialize();
        mQueueManager.Initialize();
        mUploadManager.Initialize();
        mDescriptorManager.Initialize();
        mCommandListManager.Initialize();
        mQueryManager.Initialize();
    }

    void DX12Device::Shutdown()
    {
        mQueryManager.Shutdown();
        mCommandListManager.Shutdown();
        mDescriptorManager.Shutdown();
        mUploadManager.Shutdown();
        mQueueManager.Shutdown();
        mMemoryAllocator.Shutdown();

        mDevice = nullptr;
        mAdapter = nullptr;
    }

    bool DX12Device::CheckFeatureRequirements()
    {
        bool res = true;
        
        // SM 6.6 (Required by bindless)
        if (mFeatureSupport.HighestShaderModel() < D3D_SHADER_MODEL_6_6)
        {
            int major = mFeatureSupport.HighestShaderModel() >> 4;
            int minor = mFeatureSupport.HighestShaderModel() & 0x0F;
            CUBE_LOG(Warning, DX12, "Device {0} does not support Shader Model 6.6 (Maximum: {1}.{2}), which is required.", WindowsStringView(mAdapterDesc.Description), major, minor);
            res = false;
        }

        // Resource Binding Tier 3 (Required by bindless)
        if (mFeatureSupport.ResourceBindingTier() < D3D12_RESOURCE_BINDING_TIER_3)
        {
            CUBE_LOG(Warning, DX12, "Device {0} does not support Resource Binding Tier 3 (Maximum: {1}), which is required.", WindowsStringView(mAdapterDesc.Description), (int)mFeatureSupport.ResourceBindingTier());
            res = false;
        }

        return res;
    }
} // namespace cube
