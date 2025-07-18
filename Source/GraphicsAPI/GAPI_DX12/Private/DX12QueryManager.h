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
        static constexpr int MAX_NUM_TIMESTAMP = 256;

    public:
        DX12QueryManager(DX12Device& device);

        DX12QueryManager(const DX12QueryManager& other) = delete;
        DX12QueryManager& operator=(const DX12QueryManager& rhs) = delete;

        void Initialize(Uint32 numGPUSync);
        void Shutdown();

        void SetNumGPUSync(Uint32 newNumGPUSync);
        void MoveToNextIndex(Uint64 nextGPUFrame);

        gapi::TimestampList GetLastTimestampList() const { return mLastTimestampList; }

        ID3D12QueryHeap* GetCurrentTimestampHeap() const { return mTimestampHeaps[mCurrentIndex].Get(); }
        int AddTimestamp(const String& name);
        void ResolveTimestampQueryData(ID3D12GraphicsCommandList* commandList);

    private:
        void UpdateLastTimestamp(Uint64 gpuFrame);

        DX12Device& mDevice;

        Uint32 mCurrentIndex;

        Vector<ComPtr<ID3D12QueryHeap>> mTimestampHeaps;
        DX12Allocation mTimestampGPUBuffer;

        Array<Uint64, MAX_NUM_TIMESTAMP> mLastTimestampCPUBuffer;
        gapi::TimestampList mLastTimestampList;

        Vector<Vector<String>> mTimestampNames;
    };
} // namespace cube
