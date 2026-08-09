#pragma once

#include "CoreHeader.h"

#include "GAPI.h"
#include "GAPI_CommandList.h"
#include "GAPI_Fence.h"
#include "GAPI_Resource.h"

#include <queue>

namespace cube
{
    struct UploadDesc
    {
        void* pData = nullptr;
        Uint64 size = 0;

        SharedPtr<gapi::Buffer> dstBuffer = nullptr;
        SharedPtr<gapi::Texture> dstTexture = nullptr;

        int pageId = -1;
        Uint64 offsetInPage = 0;

        bool IsValid() const { return pData != nullptr; }
        bool IsDirectWrite() const { return pageId == -1; }
    };

    // TODO: shrink page?
    class UploadManager
    {
    public:
        UploadManager() = default;

        UploadManager(const UploadManager& other) = delete;
        UploadManager& operator=(const UploadManager& rhs) = delete;

        void Initialize(GAPI* gAPI);
        void Shutdown();

        UploadDesc Allocate(SharedPtr<gapi::Buffer> dstBuffer, bool directIfPossible = false);
        UploadDesc Allocate(SharedPtr<gapi::Texture> dstTexture, bool directIfPossible = false);

        void SubmitToCopyQueue(UploadDesc& desc, SharedPtr<gapi::Fence> finishSignalFence = nullptr, Uint64 signalValue = 0);
        void Submit(UploadDesc& desc, SharedPtr<gapi::CommandList> commandList, SharedPtr<gapi::Fence> finishSignalFence = nullptr, Uint64 signalValue = 0);
        void Discard(UploadDesc& desc);

    private:
        struct Page
        {
            int refCount;

            SharedPtr<gapi::Buffer> stagingBuffer;
            void* pMappedData;
            Uint64 size;
            Uint64 offset;
        };

        static constexpr Uint64 TEXTURE_DATA_ALIGNMENT = 512;

        UploadDesc AllocateInternal(Uint64 size, Uint64 alignment);
        int AllocateNewPage(Uint64 size);
        void ReleaseAllocation(int pageId);

        void UpdateStates();
        int GetAvailableCommandListIndex();

        GAPI* mGAPI;

        Uint64 mMinPageSize;
        int mPageNextId;
        Map<int, Page> mPages;
        MultiMap<Uint64, int> mPageRemainSizeMap;

        static constexpr int MAX_COMMAND_LIST_SIZE = 2;
        struct CopyCommandList
        {
            SharedPtr<gapi::CommandList> commandList;
            Uint64 lastFenceValue = 0;
        };
        Array<CopyCommandList, MAX_COMMAND_LIST_SIZE> mCopyCommandLists;

        SharedPtr<gapi::Fence> mFence;
        Uint64 mLastFenceValue;
        std::queue<std::pair<Uint64, int>> mFenceValueAndPageIdPairQueue;
    };
} // namespace cube
