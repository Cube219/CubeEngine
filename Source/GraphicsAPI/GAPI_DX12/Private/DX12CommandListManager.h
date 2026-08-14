#pragma once

#include "DX12Header.h"

#include "DX12FenceWrapper.h"

namespace cube
{
    class DX12APIObject;
    class DX12Device;

    class DX12CommandListManager
    {
    public:
        DX12CommandListManager(DX12Device& device);

        DX12CommandListManager(const DX12CommandListManager& other) = delete;
        DX12CommandListManager& operator=(const DX12CommandListManager& rhs) = delete;

        void Initialize(Uint32 numGPUSync);
        void Shutdown();

        void SetNumGPUSync(Uint32 newNumGPUSync);
        void MoveToNextIndex(Uint64 nextGPUFrame);
        void ClearAll();

        void AddBoundObjects(ArrayView<SharedPtr<DX12APIObject>> objects);
        ID3D12CommandAllocator* GetCurrentAllocator() const { return mAllocators[mCurrentIndex].Get(); }

        void UpdateCopyStates();
        ID3D12CommandAllocator* GetCurrentCopyAllocator();
        void AddCopyBoundObjects(ArrayView<SharedPtr<DX12APIObject>> objects);
        void SignalCopyFence();

    private:
        static constexpr Uint32 NUM_COPY_ALLOCATORS = 2;

        struct CopyAllocator
        {
            ComPtr<ID3D12CommandAllocator> allocator;
            Uint64 lastFenceValue = 0;
            Vector<SharedPtr<DX12APIObject>> boundObjectsInCommand;
        };

        DX12Device& mDevice;

        Uint32 mCurrentIndex;

        Vector<ComPtr<ID3D12CommandAllocator>> mAllocators;
        Vector<Vector<SharedPtr<DX12APIObject>>> mBoundObjectsInCommand;

        Array<CopyAllocator, NUM_COPY_ALLOCATORS> mCopyAllocators;
        Uint32 mCurrentCopyAllocatorIndex;
        DX12FenceWrapper mCopyFence;
        Uint64 mCopyFenceLastValue;
    };
} // namespace cube
