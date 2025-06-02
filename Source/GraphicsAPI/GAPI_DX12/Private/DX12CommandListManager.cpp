#include "DX12CommandListManager.h"

#include "DX12APIObject.h"
#include "DX12Device.h"
#include "DX12Utility.h"

namespace cube
{
    DX12CommandListManager::DX12CommandListManager(DX12Device& device) :
        mDevice(device),
        mFence(device)
    {
        mCurrentIndex = 0;
        mLastFenceValue = 0;
        mFenceValues = {};
    }

    void DX12CommandListManager::Initialize()
    {
        int index = 0;
        for (ComPtr<ID3D12CommandAllocator>& allocator : mAllocators)
        {
            CHECK_HR(mDevice.GetDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator)));
            SET_DEBUG_NAME_FORMAT(allocator, "CommandList allocator {}", index);
            index++;
        }

        mFence.Initialize();
    }

    void DX12CommandListManager::Shutdown()
    {
        WaitCurrentAllocatorIsReady();
        mFence.Shutdown();

        for (Vector<SharedPtr<DX12APIObject>>& boundObjects : mBoundObjectsInCommand)
        {
            boundObjects.clear();
        }

        for (ComPtr<ID3D12CommandAllocator>& allocator : mAllocators)
        {
            allocator = nullptr;
        }
    }

    void DX12CommandListManager::WaitCurrentAllocatorIsReady()
    {
        mFence.Wait(mFenceValues[mCurrentIndex]);
    }

    void DX12CommandListManager::Reset()
    {
        CHECK_HR(mAllocators[mCurrentIndex]->Reset());

        mBoundObjectsInCommand[mCurrentIndex].clear();
    }

    void DX12CommandListManager::AddBoundObjects(ArrayView<SharedPtr<DX12APIObject>> objects)
    {
        mBoundObjectsInCommand[mCurrentIndex].assign(objects.begin(), objects.end());
    }

    ID3D12CommandAllocator* DX12CommandListManager::GetCurrentAllocator()
    {
        return mAllocators[mCurrentIndex].Get();
    }

    void DX12CommandListManager::MoveToNextAllocator()
    {
        mFenceValues[mCurrentIndex] = mLastFenceValue + 1;
        mFence.Signal(mDevice.GetQueueManager().GetMainQueue(), mFenceValues[mCurrentIndex]);

        mLastFenceValue++;
        mCurrentIndex = (mCurrentIndex + 1) % MAX_ALLOCATOR_SIZE;
    }
} // namespace cube
