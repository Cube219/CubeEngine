#pragma once

#include "CoreHeader.h"

#include "GAPI.h"
#include "GAPI_CommandList.h"
#include "GAPI_Fence.h"
#include "GAPI_Resource.h"

#include <queue>

namespace cube
{
    class RGBuilder;

    struct UploadAllocationInfo
    {
        Uint64 offset = 0;
        Uint64 size = std::numeric_limits<Uint64>::max();
        // TODO: Subresource uploading in texture.

        bool directIfPossible = false;
    };

    struct UploadDesc
    {
        void* pData = nullptr;
        Uint64 size = 0;

        SharedPtr<gapi::Buffer> dstBuffer = nullptr;
        SharedPtr<gapi::Texture> dstTexture = nullptr;
        Uint64 dstOffset = 0;

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

        UploadDesc Allocate(SharedPtr<gapi::Buffer> dstBuffer, const UploadAllocationInfo& info);
        UploadDesc Allocate(SharedPtr<gapi::Texture> dstTexture, const UploadAllocationInfo& info);

        Uint64 SubmitToCopyQueue(UploadDesc& desc);
        Uint64 Submit(UploadDesc& desc, gapi::CommandList* commandList);
        void Discard(UploadDesc& desc);

        SharedPtr<gapi::Fence> GetFinishFence() const { return mFinishFence; }

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
        bool IsCopyCommandListNeeded(const UploadDesc& desc) const;

        GAPI* mGAPI;
        bool mSupportsDirectMapInBuffer;
        bool mSupportsDirectMapInTexture;
        bool mNeedsToOptimizeTextureContentsUsingCommandList;

        Uint64 mMinPageSize;
        int mPageNextId;
        Map<int, Page> mPages;
        MultiMap<Uint64, int> mPageRemainSizeMap;

        SharedPtr<gapi::CommandList> mCopyCommandList;

        SharedPtr<gapi::Fence> mFinishFence;
        Uint64 mLastFinishFenceValue = 0;
        std::queue<std::pair<Uint64, int>> mFenceValueAndPageIdPairQueue;
    };
} // namespace cube
