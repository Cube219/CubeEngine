#include "DX12QueryManager.h"

#include "DX12Device.h"
#include "DX12Utility.h"

namespace cube
{
    DX12QueryManager::DX12QueryManager(DX12Device& device) :
        mDevice(device)
    {
    }

    void DX12QueryManager::Initialize(Uint32 numGPUSync)
    {
        SetNumGPUSync(numGPUSync);

        mLastTimestampCPUBuffer = {};
        mLastTimestampList.frame = 0;
        mLastTimestampList.frequency = 0;
    }

    void DX12QueryManager::Shutdown()
    {
        mDevice.GetMemoryAllocator().Free(mTimestampGPUBuffer);

        SetNumGPUSync(0);
    }

    void DX12QueryManager::SetNumGPUSync(Uint32 newNumGPUSync)
    {
        // Heap
        mTimestampHeaps.clear();
        mTimestampHeaps.resize(newNumGPUSync);

        const D3D12_QUERY_HEAP_DESC heapDesc = {
            .Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP,
            .Count = MAX_NUM_TIMESTAMP,
            .NodeMask = 0
        };
        for (Uint32 i = 0; i < newNumGPUSync; ++i)
        {
            ComPtr<ID3D12QueryHeap>& heap = mTimestampHeaps[i];

            CHECK_HR(mDevice.GetDevice()->CreateQueryHeap(&heapDesc, IID_PPV_ARGS(&heap)));
            SET_DEBUG_NAME_FORMAT(heap, "QueryHeap[{0}]", i);
        }

        // Buffer
        if (mTimestampGPUBuffer.allocation != nullptr)
        {
            mDevice.GetMemoryAllocator().Free(mTimestampGPUBuffer);
        }
        if (newNumGPUSync > 0)
        {
            const D3D12_RESOURCE_DESC bufferDesc = {
                .Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
                .Alignment = 0,
                .Width = sizeof(Uint64) * MAX_NUM_TIMESTAMP * newNumGPUSync,
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
        }

        // Name
        mTimestampNames.clear();
        mTimestampNames.resize(newNumGPUSync);
    }

    void DX12QueryManager::MoveToNextIndex(Uint64 nextGPUFrame)
    {
        Uint32 numSyncBuffer = mTimestampHeaps.size();

        mCurrentIndex = (mCurrentIndex + 1) % numSyncBuffer;

        if (nextGPUFrame >= numSyncBuffer)
        {
            UpdateLastTimestamp(nextGPUFrame - numSyncBuffer);
        }

        mTimestampNames[mCurrentIndex].clear();
    }

    int DX12QueryManager::AddTimestamp(const String& name)
    {
        // TODO: Add critical section?
        mTimestampNames[mCurrentIndex].push_back(name);
        const int currentSize = static_cast<int>(mTimestampNames[mCurrentIndex].size());

        CHECK(currentSize <= MAX_NUM_TIMESTAMP);

        return currentSize - 1;
    }

    void DX12QueryManager::ResolveTimestampQueryData(ID3D12GraphicsCommandList* commandList)
    {
        const int currentCount = static_cast<int>(mTimestampNames[mCurrentIndex].size());
        const Uint64 bufferSubSize = sizeof(Uint64) * MAX_NUM_TIMESTAMP;

        commandList->ResolveQueryData(GetCurrentTimestampHeap(), D3D12_QUERY_TYPE_TIMESTAMP, 0, currentCount,
            mTimestampGPUBuffer.allocation->GetResource(), bufferSubSize * mCurrentIndex);
    }

    
    void DX12QueryManager::UpdateLastTimestamp(Uint64 gpuFrame)
    {
        // Current index will be the last index.
        // This function will be called after moving to next index so it is guaranteed
        // that the last GPU sync related to the index is finished in DX12Device::BeginGPUFrame().
        const Uint32 lastIndex = mCurrentIndex;
        const Uint64 bufferSubSize = sizeof(Uint64) * MAX_NUM_TIMESTAMP;

        mTimestampGPUBuffer.Map(bufferSubSize * lastIndex, bufferSubSize * (lastIndex + 1));
        memcpy(mLastTimestampCPUBuffer.data(), (char*)mTimestampGPUBuffer.pMapPtr + (lastIndex * bufferSubSize), bufferSubSize);
        mTimestampGPUBuffer.Unmap();

        const Vector<String>& lastTimestampNames = mTimestampNames[lastIndex];
        mLastTimestampList.frame = gpuFrame;
        mDevice.GetQueueManager().GetMainQueue()->GetTimestampFrequency(&mLastTimestampList.frequency);
        mLastTimestampList.timestamps.resize(lastTimestampNames.size());
        for (int i = 0; i < lastTimestampNames.size()   ; ++i)
        {
            mLastTimestampList.timestamps[i].name = lastTimestampNames[i];
            mLastTimestampList.timestamps[i].time = mLastTimestampCPUBuffer[i];
        }
    }
} // namespace cube
