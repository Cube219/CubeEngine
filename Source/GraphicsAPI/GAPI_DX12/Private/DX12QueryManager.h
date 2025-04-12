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
        static constexpr int MAX_HEAP_SIZE = 5;
        static constexpr int MAX_NUM_TIMESTAMP = 256;

    public:
        DX12QueryManager(DX12Device& device);

        DX12QueryManager(const DX12QueryManager& other) = delete;
        DX12QueryManager& operator=(const DX12QueryManager& rhs) = delete;

        void Initialize();
        void Shutdown();

        void WaitCurrentHeapIsReady();
        void ClearCurrentTimestampNames();
        void MoveToNextHeap();
        void UpdateLastTimestamp();

        gapi::TimestampList GetLastTimestampList() const { return mLastTimestampList; }

        ID3D12QueryHeap* GetCurrentTimestampHeap() const { return mTimestampHeaps[mCurrentFrame % MAX_HEAP_SIZE].Get(); }
        int AddTimestamp(const String& name);
        void ResolveTimestampQueryData(ID3D12GraphicsCommandList* commandList);

    private:
        DX12Device& mDevice;

        Uint32 mCurrentFrame;
        DX12Fence mFence;
        Array<Uint64, MAX_HEAP_SIZE> mFenceValues;

        Array<ComPtr<ID3D12QueryHeap>, MAX_HEAP_SIZE> mTimestampHeaps;
        DX12Allocation mTimestampGPUBuffer;

        Array<Uint64, MAX_NUM_TIMESTAMP> mLastTimestampCPUBuffer;
        Uint64 mLastTimestampFrame;
        gapi::TimestampList mLastTimestampList;

        Array<Vector<String>, MAX_HEAP_SIZE> mTimestampNames;
    };
} // namespace cube
