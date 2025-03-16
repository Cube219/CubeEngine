#include "DX12CommandListManager.h"

#include "DX12Device.h"
#include "DX12Utility.h"

namespace cube
{
    DX12CommandListManager::DX12CommandListManager(DX12Device& device) :
        mDevice(device),
        mFence(device)
    {
        mCurrentIndex = 0;
        mFenceValues = {};
    }

    void DX12CommandListManager::Initialize()
    {
        for (ComPtr<ID3D12CommandAllocator>& allocator : mAllocators)
        {
            CHECK_HR(mDevice.GetDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator)));
        }

        mFence.Initialize();
    }

    void DX12CommandListManager::Shutdown()
    {
        WaitCurrentAllocatorIsReady();
        mFence.Shutdown();

        for (ComPtr<ID3D12CommandAllocator>& allocator : mAllocators)
        {
            allocator = nullptr;
        }
    }

    void DX12CommandListManager::WaitCurrentAllocatorIsReady()
    {
        mFence.Wait(mFenceValues[mCurrentIndex % MAX_ALLOCATOR_SIZE]);
    }

    void DX12CommandListManager::Reset()
    {
        CHECK_HR(mAllocators[mCurrentIndex % MAX_ALLOCATOR_SIZE]->Reset());
    }

    ID3D12CommandAllocator* DX12CommandListManager::GetCurrentAllocator()
    {
        return mAllocators[mCurrentIndex % MAX_ALLOCATOR_SIZE].Get();
    }

    void DX12CommandListManager::MoveToNextAllocator()
    {
        mFenceValues[mCurrentIndex % MAX_ALLOCATOR_SIZE] = mCurrentIndex + 1;
        mFence.Signal(mDevice.GetQueueManager().GetMainQueue(), mFenceValues[mCurrentIndex % MAX_ALLOCATOR_SIZE]);

        mCurrentIndex++;
    }
} // namespace cube
