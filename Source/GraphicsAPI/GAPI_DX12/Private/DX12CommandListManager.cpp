#include "DX12CommandListManager.h"

#include "DX12APIObject.h"
#include "DX12Device.h"
#include "DX12Utility.h"

namespace cube
{
    DX12CommandListManager::DX12CommandListManager(DX12Device& device)
        : mDevice(device)
        , mCopyFence(device)
    {
        mCurrentIndex = 0;
        mCurrentCopyAllocatorIndex = 0;
        mCopyFenceLastValue = 0;
    }

    void DX12CommandListManager::Initialize(Uint32 numGPUSync)
    {
        SetNumGPUSync(numGPUSync);

        mCopyFence.Initialize(CUBE_T("CopyCommandListFence"), true);

        for (Uint32 i = 0; i < NUM_COPY_ALLOCATORS; ++i)
        {
            CopyAllocator& copyAllocator = mCopyAllocators[i];
            CHECK_HR(mDevice.GetDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY, IID_PPV_ARGS(&copyAllocator.allocator)));
            SET_DEBUG_NAME_FORMAT(copyAllocator.allocator, "CopyCommandListAllocator[{0}]", i);
            copyAllocator.lastFenceValue = 0;
        }
    }

    void DX12CommandListManager::Shutdown()
    {
        mCopyFence.Shutdown();

        for (Uint32 i = 0; i < NUM_COPY_ALLOCATORS; ++i)
        {
            mCopyAllocators[i].allocator = nullptr;
            mCopyAllocators[i].boundObjectsInCommand.clear();
        }

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

        mBoundObjectsInCommand.clear();
        mBoundObjectsInCommand.resize(newNumGPUSync);
    }

    void DX12CommandListManager::MoveToNextIndex(Uint64 nextGPUFrame)
    {
        mCurrentIndex = (mCurrentIndex + 1) % mAllocators.size();

        CHECK_HR(mAllocators[mCurrentIndex]->Reset());
        mBoundObjectsInCommand[mCurrentIndex].clear();
    }

    void DX12CommandListManager::ClearAll()
    {
        for (auto& allocator : mAllocators)
        {
            CHECK_HR(allocator->Reset());
        }

        for (auto& boundObjects : mBoundObjectsInCommand)
        {
            boundObjects.clear();
        }
    }

    void DX12CommandListManager::AddBoundObjects(ArrayView<SharedPtr<DX12APIObject>> objects)
    {
        auto& boundObjects = mBoundObjectsInCommand[mCurrentIndex];
        boundObjects.insert(boundObjects.end(), objects.begin(), objects.end());
    }

    void DX12CommandListManager::UpdateCopyStates()
    {
        Uint64 completedFenceValue = mCopyFence.GetCompletedValue();

        // Reset the command list allocator if it's all command lists are executed.
        for (Uint32 i = 0; i < NUM_COPY_ALLOCATORS; ++i)
        {
            CopyAllocator& copyAllocator = mCopyAllocators[i];
            if (!copyAllocator.boundObjectsInCommand.empty() && copyAllocator.lastFenceValue <= completedFenceValue)
            {
                CHECK_HR(copyAllocator.allocator->Reset());
                copyAllocator.boundObjectsInCommand.clear();
            }
        }

        // Change the allocator if empty allocator is existed.
        for (Uint32 i = 0; i < NUM_COPY_ALLOCATORS; ++i)
        {
            if (mCopyAllocators[i].lastFenceValue <= completedFenceValue)
            {
                mCurrentCopyAllocatorIndex = i;
                break;
            }
        }
    }

    ID3D12CommandAllocator* DX12CommandListManager::GetCurrentCopyAllocator()
    {
        UpdateCopyStates();
        return mCopyAllocators[mCurrentCopyAllocatorIndex].allocator.Get();
    }

    void DX12CommandListManager::AddCopyBoundObjects(ArrayView<SharedPtr<DX12APIObject>> objects)
    {
        auto& boundObjects = mCopyAllocators[mCurrentCopyAllocatorIndex].boundObjectsInCommand;
        boundObjects.insert(boundObjects.end(), objects.begin(), objects.end());
    }

    void DX12CommandListManager::SignalCopyFence()
    {
        mCopyFenceLastValue++;
        mCopyFence.SignalToQueue(mDevice.GetQueueManager().GetCopyQueue(), mCopyFenceLastValue);
        mCopyAllocators[mCurrentCopyAllocatorIndex].lastFenceValue = mCopyFenceLastValue;
    }
} // namespace cube
