#include "DX12CommandListManager.h"

#include "DX12APIObject.h"
#include "DX12Device.h"
#include "DX12Utility.h"

namespace cube
{
    DX12CommandListManager::DX12CommandListManager(DX12Device& device)
        : mDevice(device)
    {
        mCurrentIndex = 0;
    }

    void DX12CommandListManager::Initialize(Uint32 numGPUSync)
    {
        SetNumGPUSync(numGPUSync);
        int index = 0;
        for (ComPtr<ID3D12CommandAllocator>& allocator : mAllocators)
        {
            CHECK_HR(mDevice.GetDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator)));
            SET_DEBUG_NAME_FORMAT(allocator, "CommandListAllocator[{0}]", index);
            index++;
        }
    }

    void DX12CommandListManager::Shutdown()
    {
        SetNumGPUSync(0);
    }

    void DX12CommandListManager::SetNumGPUSync(Uint32 newNumGPUSync)
    {
        mAllocators.clear();
        mAllocators.resize(newNumGPUSync);
        for (Uint32 i = 0; i < newNumGPUSync; ++i)
        {
            ComPtr<ID3D12CommandAllocator>& allocator = mAllocators[i];

            CHECK_HR(mDevice.GetDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator)));
            SET_DEBUG_NAME_FORMAT(allocator, "CommandListAllocator[{0}]", i);
        }
    }

    void DX12CommandListManager::MoveToNextIndex(Uint64 nextGPUFrame)
    {
        mCurrentIndex = (mCurrentIndex + 1) % mAllocators.size();

        CHECK_HR(mAllocators[mCurrentIndex]->Reset());
    }
} // namespace cube
