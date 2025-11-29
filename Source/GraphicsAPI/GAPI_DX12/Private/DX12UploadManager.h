#pragma once

#include "DX12Header.h"

#include <queue>

#include "DX12Fence.h"
#include "DX12MemoryAllocator.h"

#include "GAPI_Resource.h"

namespace cube
{
    class DX12APIObject;
    class DX12Device;

    struct DX12UploadDesc
    {
        gapi::ResourceType type;

        void* pData = nullptr;
        Uint64 size;

        ID3D12Resource* dstResource;
        DX12APIObject* dstAPIObject;
        D3D12_PLACED_SUBRESOURCE_FOOTPRINT dstTextureLayout;

        int pageId;
        Uint64 offsetInPage;

        bool IsValid() const { return pData != nullptr; }
    };

    // TODO: shrink page?
    class DX12UploadManager
    {
    public:
        DX12UploadManager(DX12Device& device);

        DX12UploadManager(const DX12UploadManager& other) = delete;
        DX12UploadManager operator=(const DX12UploadManager& rhs) = delete;

        void Initialize(Uint64 minPageSize = 4ll * 1024 * 1024 /* 4 MiB */);
        void Shutdown();

        // TODO: Thread-safe
        DX12UploadDesc Allocate(gapi::ResourceType type, Uint64 size, Uint64 alignment = 1);
        DX12FenceValue Submit(DX12UploadDesc& desc, bool waitForCompletion = false);
        void Discard(DX12UploadDesc& desc);

        bool IsUploadFinished(DX12FenceValue submitFenceValue);

    private:
        struct Page
        {
            int refCount;

            DX12Allocation allocation;
            Uint64 size;
            Uint64 offset;
        };

        int AllocateNewPage(Uint64 size);
        void ReleaseAllocation(int pageId);

        void UpdateStates();

        DX12Device& mDevice;

        Uint64 mMinPageSize;
        int mPageNextId;
        Map<int, Page> mPages;
        MultiMap<Uint64, int> mPageRemainSizeMap;

        static constexpr int MAX_ALLOCATOR_SIZE = 2;
        struct CommandListAllocator
        {
            ComPtr<ID3D12CommandAllocator> allocator;
            DX12FenceValue lastFenceValue;
            Vector<SharedPtr<DX12APIObject>> boundObjectsInCommand;
        };
        Array<CommandListAllocator, MAX_ALLOCATOR_SIZE> mCommandListAllocators;
        Uint32 mCurrentAllocatorIndex;

        DX12Fence mFence;
        DX12FenceValue mLastFenceValue;
        std::queue<std::pair<Uint64, int>> mFenceValueAndPageIdPairQueue;
    };
} // namespace cube
