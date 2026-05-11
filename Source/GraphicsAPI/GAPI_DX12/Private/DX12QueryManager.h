#pragma once

#include "DX12Header.h"

#include "DX12Fence.h"
#include "DX12MemoryAllocator.h"
#include "GAPI_Timestamp.h"

namespace cube
{
    class DX12Device;

    class DX12QueryManager
    {
    public:
        static constexpr int MAX_NUM_QUERY = 1024;

    public:
        DX12QueryManager(DX12Device& device);

        DX12QueryManager(const DX12QueryManager& other) = delete;
        DX12QueryManager& operator=(const DX12QueryManager& rhs) = delete;

        void Initialize(Uint32 numGPUSync);
        void Shutdown();

        void SetNumGPUSync(Uint32 newNumGPUSync);
        void MoveToNextGPUSync(Uint64 nextGPUFrame);

        gapi::TimestampRangeList GetLastTimestampRangeList() const { return mLastTimestampRangeList; }

        ID3D12QueryHeap* GetCurrentTimestampHeap() const { return mTimestampHeaps[mCurrentGPUSyncIndex].Get(); }

        Uint32 GetCurrentLastQueryIndexAndUse(Uint32 numUseQueries);
        void AddTimestampRange(StringView name, Uint32 beginQueryIndex, Uint32 endQueryIndex);
        void ResolveQueryData(ID3D12GraphicsCommandList* commandList);

    private:
        void UpdateLastTimestamp(Uint64 gpuFrame);

        DX12Device& mDevice;

        Uint32 mCurrentGPUSyncIndex;

        Vector<ComPtr<ID3D12QueryHeap>> mTimestampHeaps;
        Vector<Uint32> mLastQueryIndices;
        DX12Allocation mQueryGPUBuffer;

        Array<Uint64, MAX_NUM_QUERY> mLastQueryCPUBuffer;
        gapi::TimestampRangeList mLastTimestampRangeList;

        struct DX12TimestampRange
        {
            String name;
            Uint32 beginQueryIndex;
            Uint32 endQueryIndex;
        };
        Vector<Vector<DX12TimestampRange>> mTimestampRanges;
    };
} // namespace cube
