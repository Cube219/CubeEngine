#include "DX12QueryManager.h"

#include "DX12Device.h"
#include "DX12Utility.h"

namespace cube
{
    DX12QueryManager::DX12QueryManager(DX12Device& device) :
        mDevice(device),
        mCurrentFrame(0),
        mFence(device)
    {
    }

    void DX12QueryManager::Initialize()
    {
        const D3D12_QUERY_HEAP_DESC heapDesc = {
            .Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP,
            .Count = MAX_NUM_TIMESTAMP,
            .NodeMask = 0
        };
        for (ComPtr<ID3D12QueryHeap>& heap : mTimestampHeaps)
        {
            CHECK_HR(mDevice.GetDevice()->CreateQueryHeap(&heapDesc, IID_PPV_ARGS(&heap)));
        }
        const D3D12_RESOURCE_DESC bufferDesc = {
            .Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
            .Alignment = 0,
            .Width = sizeof(Uint64) * MAX_NUM_TIMESTAMP * MAX_HEAP_SIZE,
            .Height = 1,
            .DepthOrArraySize = 1,
            .MipLevels = 1,
            .Format = DXGI_FORMAT_UNKNOWN,
            .SampleDesc = {
                .Count = 1,
                .Quality = 0 },
            .Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
            .Flags = D3D12_RESOURCE_FLAG_NONE
        };
        mTimestampGPUBuffer = mDevice.GetMemoryAllocator().Allocate(D3D12_HEAP_TYPE_READBACK, bufferDesc);

        mFence.Initialize();
        mFenceValues = {};
        mCurrentFrame = 1;

        mLastTimestampCPUBuffer = {};
        mLastTimestampFrame = 0;
        mLastTimestampList.frame = 0;
        mLastTimestampList.frequency = 0;
    }

    void DX12QueryManager::Shutdown()
    {
        mFence.Shutdown();

        mDevice.GetMemoryAllocator().Free(mTimestampGPUBuffer);

        for (ComPtr<ID3D12QueryHeap>& heap : mTimestampHeaps)
        {
            heap = nullptr;
        }
    }

    void DX12QueryManager::WaitCurrentHeapIsReady()
    {
        mFence.Wait(mFenceValues[mCurrentFrame % MAX_HEAP_SIZE]);
        UpdateLastTimestamp();
    }

    void DX12QueryManager::ClearCurrentTimestampNames()
    {
        mTimestampNames[mCurrentFrame % MAX_HEAP_SIZE].clear();
    }

    void DX12QueryManager::MoveToNextHeap()
    {
        mFenceValues[mCurrentFrame % MAX_HEAP_SIZE] = mCurrentFrame + 1;
        mFence.Signal(mDevice.GetQueueManager().GetMainQueue(), mFenceValues[mCurrentFrame % MAX_HEAP_SIZE]);

        mCurrentFrame++;
    }

    void DX12QueryManager::UpdateLastTimestamp()
    {
        const Uint64 compValue = mFence.GetCompletedValue();
        if (compValue > mLastTimestampFrame)
        {
            mLastTimestampFrame = compValue;

            const Uint32 lastIndex = (mLastTimestampFrame % MAX_HEAP_SIZE);
            const Uint64 bufferSubSize = sizeof(Uint64) * MAX_NUM_TIMESTAMP;

            mTimestampGPUBuffer.Map(bufferSubSize * lastIndex, bufferSubSize * (lastIndex + 1));
            memcpy(mLastTimestampCPUBuffer.data(), (char*)mTimestampGPUBuffer.pMapPtr + (lastIndex * bufferSubSize), bufferSubSize);
            mTimestampGPUBuffer.Unmap();

            const Vector<String>& lastTimestampNames = mTimestampNames[lastIndex];
            mLastTimestampList.frame = mLastTimestampFrame;
            mDevice.GetQueueManager().GetMainQueue()->GetTimestampFrequency(&mLastTimestampList.frequency);
            mLastTimestampList.timestamps.resize(lastTimestampNames.size());
            for (int i = 0; i < lastTimestampNames.size()   ; ++i)
            {
                mLastTimestampList.timestamps[i].name = lastTimestampNames[i];
                mLastTimestampList.timestamps[i].time = mLastTimestampCPUBuffer[i];
            }
        }
    }

    int DX12QueryManager::AddTimestamp(const String& name)
    {
        // TODO: Added critical section?
        mTimestampNames[mCurrentFrame % MAX_HEAP_SIZE].push_back(name);
        const int currentSize = static_cast<int>(mTimestampNames[mCurrentFrame % MAX_HEAP_SIZE].size());

        CHECK(currentSize <= MAX_NUM_TIMESTAMP);

        return currentSize - 1;
    }

    void DX12QueryManager::ResolveTimestampQueryData(ID3D12GraphicsCommandList* commandList)
    {
        const int currentIndex = mCurrentFrame % MAX_HEAP_SIZE;
        const int currentCount = static_cast<int>(mTimestampNames[currentIndex].size());
        const Uint64 bufferSubSize = sizeof(Uint64) * MAX_NUM_TIMESTAMP;

        commandList->ResolveQueryData(GetCurrentTimestampHeap(), D3D12_QUERY_TYPE_TIMESTAMP, 0, currentCount,
            mTimestampGPUBuffer.allocation->GetResource(), bufferSubSize * currentIndex);
    }
} // namespace cube
