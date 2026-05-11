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

        mLastQueryCPUBuffer = {};
        mLastTimestampRangeList.frame = 0;
        mLastTimestampRangeList.frequency = 0;
    }

    void DX12QueryManager::Shutdown()
    {
        mDevice.GetMemoryAllocator().Free(mQueryGPUBuffer);

        SetNumGPUSync(0);
    }

    void DX12QueryManager::SetNumGPUSync(Uint32 newNumGPUSync)
    {
        // Heap
        mTimestampHeaps.clear();
        mTimestampHeaps.resize(newNumGPUSync);

        const D3D12_QUERY_HEAP_DESC heapDesc = {
            .Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP,
            .Count = MAX_NUM_QUERY,
            .NodeMask = 0
        };
        for (Uint32 i = 0; i < newNumGPUSync; ++i)
        {
            ComPtr<ID3D12QueryHeap>& heap = mTimestampHeaps[i];

            CHECK_HR(mDevice.GetDevice()->CreateQueryHeap(&heapDesc, IID_PPV_ARGS(&heap)));
            SET_DEBUG_NAME_FORMAT(heap, "QueryHeap[{0}]", i);
        }

        mLastQueryIndices.clear();
        mLastQueryIndices.resize(newNumGPUSync, 0);

        // Buffer
        if (mQueryGPUBuffer.resource != nullptr)
        {
            mDevice.GetMemoryAllocator().Free(mQueryGPUBuffer);
        }
        if (newNumGPUSync > 0)
        {
            const D3D12_RESOURCE_DESC bufferDesc = {
                .Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
                .Alignment = 0,
                .Width = sizeof(Uint64) * MAX_NUM_QUERY * newNumGPUSync,
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
            mQueryGPUBuffer = mDevice.GetMemoryAllocator().Allocate(D3D12_HEAP_TYPE_READBACK, bufferDesc);
            SET_DEBUG_NAME(mQueryGPUBuffer.resource, "Timestamp GPU buffer");
        }

        // Timestamp ranges
        mTimestampRanges.clear();
        mTimestampRanges.resize(newNumGPUSync);

        mCurrentGPUSyncIndex = 0;
    }

    void DX12QueryManager::MoveToNextGPUSync(Uint64 nextGPUFrame)
    {
        Uint32 numSyncBuffer = mTimestampHeaps.size();

        mCurrentGPUSyncIndex = (mCurrentGPUSyncIndex + 1) % numSyncBuffer;

        if (nextGPUFrame >= numSyncBuffer)
        {
            UpdateLastTimestamp(nextGPUFrame - numSyncBuffer);
        }

        mTimestampRanges[mCurrentGPUSyncIndex].clear();
        mLastQueryIndices[mCurrentGPUSyncIndex] = 0;
    }

    Uint32 DX12QueryManager::GetCurrentLastQueryIndexAndUse(Uint32 numUseQueries)
    {
        Uint32 res = mLastQueryIndices[mCurrentGPUSyncIndex];

        mLastQueryIndices[mCurrentGPUSyncIndex] += numUseQueries;
        CHECK(mLastQueryIndices[mCurrentGPUSyncIndex] <= MAX_NUM_QUERY);

        return res;
    }

    void DX12QueryManager::AddTimestampRange(StringView name, Uint32 beginQueryIndex, Uint32 endQueryIndex)
    {
        mTimestampRanges[mCurrentGPUSyncIndex].push_back({
            .name = { name.begin(), name.end() },
            .beginQueryIndex = beginQueryIndex,
            .endQueryIndex = endQueryIndex
        });
    }

    void DX12QueryManager::ResolveQueryData(ID3D12GraphicsCommandList* commandList)
    {
        const Uint32 numQueries = mLastQueryIndices[mCurrentGPUSyncIndex];
        const Uint64 bufferSubSize = sizeof(Uint64) * MAX_NUM_QUERY;

        commandList->ResolveQueryData(GetCurrentTimestampHeap(), D3D12_QUERY_TYPE_TIMESTAMP, 0, numQueries,
            mQueryGPUBuffer.resource, bufferSubSize * mCurrentGPUSyncIndex);
    }

    void DX12QueryManager::UpdateLastTimestamp(Uint64 gpuFrame)
    {
        // Current index is the last index.
        // This function will be called after moving to next GPU sync index so it is guaranteed
        // that the last GPU sync is finished in DX12Device::BeginGPUFrame().
        const Uint32 lastIndex = mCurrentGPUSyncIndex;
        const Uint64 bufferSubSize = sizeof(Uint64) * MAX_NUM_QUERY;

        mQueryGPUBuffer.Map(bufferSubSize * lastIndex, bufferSubSize * (lastIndex + 1));
        memcpy(mLastQueryCPUBuffer.data(), (char*)mQueryGPUBuffer.pMapPtr + (lastIndex * bufferSubSize), bufferSubSize);
        mQueryGPUBuffer.Unmap();

        const Vector<DX12TimestampRange>& lastTimestampRanges = mTimestampRanges[lastIndex];
        mLastTimestampRangeList.frame = gpuFrame;
        mDevice.GetQueueManager().GetMainQueue()->GetTimestampFrequency(&mLastTimestampRangeList.frequency);
        mLastTimestampRangeList.timestampRanges.resize(lastTimestampRanges.size());
        for (int i = 0; i < lastTimestampRanges.size(); ++i)
        {
            const DX12TimestampRange& dx12TimestampRange = lastTimestampRanges[i];

            mLastTimestampRangeList.timestampRanges[i].name = dx12TimestampRange.name;
            mLastTimestampRangeList.timestampRanges[i].beginTime = mLastQueryCPUBuffer[dx12TimestampRange.beginQueryIndex];
            mLastTimestampRangeList.timestampRanges[i].endTime = mLastQueryCPUBuffer[dx12TimestampRange.endQueryIndex];
        }
    }
} // namespace cube
