#include "UploadManager.h"

#include "Allocator/AllocatorUtility.h"
#include "Allocator/FrameAllocator.h"
#include "Checker.h"
#include "GAPI_Buffer.h"
#include "GAPI_Texture.h"

namespace cube
{
    void UploadManager::Initialize(GAPI* gAPI)
    {
        mGAPI = gAPI;
        mMinPageSize = 4ll * 1024 * 1024; // 4 MiB
        mPageNextId = 0;
        mLastFenceValue = 0;

        mFence = mGAPI->CreateFence({
            .debugName = CUBE_T("UploadManagerFence")
        });

        int index = 0;
        for (CopyCommandList& copyCommandList : mCopyCommandLists)
        {
            FrameString debugName = Format<FrameString>(CUBE_T("UploadManagerCopyCommandList_{0}"), index);
            copyCommandList.commandList = mGAPI->CreateCommandList({
                .type = gapi::CommandListType::Copy,
                .debugName = debugName
            });
            copyCommandList.lastFenceValue = 0;
            index++;
        }
    }

    void UploadManager::Shutdown()
    {
        mFence->Wait(mLastFenceValue);
        UpdateStates();
        mFence = nullptr;

        for (CopyCommandList& copyCommandList : mCopyCommandLists)
        {
            copyCommandList.commandList = nullptr;
        }

        for (auto& [_, page] : mPages)
        {
            CHECK(page.refCount == 0);
            page.stagingBuffer->Unmap();
            page.stagingBuffer = nullptr;
        }
        mPages.clear();
        mPageRemainSizeMap.clear();

        mGAPI = nullptr;
    }

    UploadDesc UploadManager::Allocate(SharedPtr<gapi::Buffer> dstBuffer)
    {
        CHECK(dstBuffer->GetUsage() == gapi::ResourceUsage::GPUOnly);

        if (mGAPI->IsDirectMapSupported(gapi::ResourceType::Buffer))
        {
            void* pData = dstBuffer->Map();
            return {
                .pData = pData,
                .size = dstBuffer->GetSize(),
                .dstBuffer = dstBuffer,
            };
        }

        UploadDesc desc = AllocateInternal(dstBuffer->GetSize(), 1);
        desc.dstBuffer = dstBuffer;
        return desc;
    }

    UploadDesc UploadManager::Allocate(SharedPtr<gapi::Texture> dstTexture)
    {
        CHECK(dstTexture->GetUsage() == gapi::ResourceUsage::GPUOnly);

        if (mGAPI->IsDirectMapSupported(gapi::ResourceType::Texture))
        {
            void* pData = dstTexture->Map();
            return {
                .pData = pData,
                .size = dstTexture->GetTotalDataSize(),
                .dstTexture = dstTexture,
            };
        }

        UploadDesc desc = AllocateInternal(dstTexture->GetTotalDataSize(), TEXTURE_DATA_ALIGNMENT);
        desc.dstTexture = dstTexture;
        return desc;
    }

    UploadDesc UploadManager::AllocateInternal(Uint64 size, Uint64 alignment)
    {
        UpdateStates();

        Uint64 alignSize = size + alignment - 1;
        auto findIt = mPageRemainSizeMap.lower_bound(alignSize);
        if (findIt == mPageRemainSizeMap.end())
        {
            AllocateNewPage(alignSize);
            findIt = mPageRemainSizeMap.lower_bound(alignSize);
        }
        CHECK(findIt != mPageRemainSizeMap.end());

        int pageId = findIt->second;
        Page& page = mPages.find(pageId)->second;
        mPageRemainSizeMap.erase(findIt);

        CHECK(page.offset + alignSize <= page.size);
        Uint64 ptr = (Uint64)page.pMappedData + page.offset;
        Uint64 alignPtr = Align(ptr, alignment);
        Uint64 alignGap = alignPtr - ptr;

        UploadDesc res = {
            .pData = (Byte*)page.pMappedData + page.offset + alignGap,
            .size = size,
            .pageId = pageId,
            .offsetInPage = page.offset + alignGap,
        };

        page.offset += alignGap + size;
        page.refCount++;
        mPageRemainSizeMap.insert({ page.size - page.offset, pageId });

        return res;
    }

    Uint64 UploadManager::Submit(UploadDesc& desc, bool waitForCompletion)
    {
        UpdateStates();

        if (desc.IsDirectWrite())
        {
            if (desc.dstBuffer)
            {
                desc.dstBuffer->Unmap();
            }
            else if (desc.dstTexture)
            {
                desc.dstTexture->Unmap();

                int commandListIndex = GetAvailableCommandListIndex();
                CopyCommandList& copyCommandList = mCopyCommandLists[commandListIndex];

                copyCommandList.commandList->Reset();
                copyCommandList.commandList->Begin();
                copyCommandList.commandList->OptimizeTextureContentsForGPUAccess(desc.dstTexture);
                copyCommandList.commandList->End();

                mLastFenceValue++;
                copyCommandList.commandList->Submit(false, mFence.get(), mLastFenceValue);
                copyCommandList.lastFenceValue = mLastFenceValue;
            }

            desc.pData = nullptr;
            desc.dstBuffer = nullptr;
            desc.dstTexture = nullptr;

            if (waitForCompletion)
            {
                mFence->Wait(mLastFenceValue);
            }

            return mLastFenceValue;
        }

        auto pageIt = mPages.find(desc.pageId);
        CHECK(pageIt != mPages.end());
        Page& page = pageIt->second;

        int commandListIndex = GetAvailableCommandListIndex();
        CopyCommandList& copyCommandList = mCopyCommandLists[commandListIndex];

        copyCommandList.commandList->Reset();
        copyCommandList.commandList->Begin();

        if (desc.dstBuffer)
        {
            copyCommandList.commandList->CopyBuffer(page.stagingBuffer, desc.offsetInPage, desc.dstBuffer, 0, desc.size);
        }
        else
        {
            // Upload the entire texture resource, so the previous contents can be discarded.
            gapi::ResourceBarrier barrier = {
                .resourceType = gapi::ResourceBarrier::ResourceType::Texture,
                .texture = desc.dstTexture,
                .subresourceIndex = -1,
                .discard = true,
                .syncSrc = gapi::ResourceSyncFlag::None,
                .syncDst = gapi::ResourceSyncFlag::Copy,
                .accessSrc = gapi::ResourceAccessFlag::NoAccess,
                .accessDst = gapi::ResourceAccessFlag::CopyDst,
                .layoutSrc = gapi::ResourceLayout::Undefined,
                .layoutDst = gapi::ResourceLayout::Common,
            };
            copyCommandList.commandList->SetResourceBarrier(barrier);

            copyCommandList.commandList->CopyBufferToTexture(page.stagingBuffer, desc.offsetInPage, desc.dstTexture);

            copyCommandList.commandList->OptimizeTextureContentsForGPUAccess(desc.dstTexture);
        }

        copyCommandList.commandList->End();

        mLastFenceValue++;
        copyCommandList.commandList->Submit(false, mFence.get(), mLastFenceValue);
        copyCommandList.lastFenceValue = mLastFenceValue;
        mFenceValueAndPageIdPairQueue.push({ mLastFenceValue, desc.pageId });

        desc.pData = nullptr;
        desc.dstBuffer = nullptr;
        desc.dstTexture = nullptr;

        if (waitForCompletion)
        {
            mFence->Wait(mLastFenceValue);
        }

        return mLastFenceValue;
    }

    void UploadManager::Discard(UploadDesc& desc)
    {
        UpdateStates();

        if (desc.IsDirectWrite())
        {
            if (desc.dstBuffer)
            {
                desc.dstBuffer->Unmap();
            }
            else if (desc.dstTexture)
            {
                desc.dstTexture->Unmap();
            }
        }
        else
        {
            auto pageIt = mPages.find(desc.pageId);
            CHECK(pageIt != mPages.end());

            // Just add fence to identify the allocation was released in UpdateStates.
            // The page can be used in other in-flight upload descs, so the release
            // must be deferred until all previously submitted uploads complete.
            int commandListIndex = GetAvailableCommandListIndex();
            CopyCommandList& copyCommandList = mCopyCommandLists[commandListIndex];

            copyCommandList.commandList->Reset();
            copyCommandList.commandList->Begin();
            copyCommandList.commandList->End();

            mLastFenceValue++;
            copyCommandList.commandList->Submit(false, mFence.get(), mLastFenceValue);
            copyCommandList.lastFenceValue = mLastFenceValue;
            mFenceValueAndPageIdPairQueue.push({ mLastFenceValue, desc.pageId });
        }

        desc.pData = nullptr;
        desc.dstBuffer = nullptr;
        desc.dstTexture = nullptr;
    }

    bool UploadManager::IsUploadFinished(Uint64 submitFenceValue)
    {
        return submitFenceValue <= mFence->GetCompletedValue();
    }

    int UploadManager::AllocateNewPage(Uint64 size)
    {
        size = std::max(size, mMinPageSize);

        FrameString debugName = Format<FrameString>(CUBE_T("UploadManagerPage_{0}"), mPageNextId);
        SharedPtr<gapi::Buffer> stagingBuffer = mGAPI->CreateBuffer({
            .usage = gapi::ResourceUsage::CPUtoGPU,
            .bufferInfo = {
                .type = gapi::BufferType::Raw,
                .size = size,
            },
            .debugName = debugName
        });

        Page newPage = {
            .refCount = 0,
            .stagingBuffer = stagingBuffer,
            .pMappedData = stagingBuffer->Map(),
            .size = size,
            .offset = 0
        };
        mPages.insert({ mPageNextId, newPage });
        mPageNextId++;

        mPageRemainSizeMap.insert({ size, mPageNextId - 1 });

        return mPageNextId - 1;
    }

    void UploadManager::ReleaseAllocation(int pageId)
    {
        auto findIt = mPages.find(pageId);
        CHECK(findIt != mPages.end());

        Page& page = findIt->second;
        CHECK(page.refCount > 0);
        page.refCount--;

        if (page.refCount == 0)
        {
            // Reset the page
            Uint64 remainSize = page.size - page.offset;
            auto findRange = mPageRemainSizeMap.equal_range(remainSize);
            auto findIt = mPageRemainSizeMap.end();
            for (auto it = findRange.first; it != findRange.second; ++it)
            {
                if (it->second == pageId)
                {
                    findIt = it;
                    break;
                }
            }
            CHECK(findIt != mPageRemainSizeMap.end());

            mPageRemainSizeMap.erase(findIt);

            page.offset = 0;
            mPageRemainSizeMap.insert({ page.size, pageId });
        }
    }

    void UploadManager::UpdateStates()
    {
        Uint64 completedFenceValue = mFence->GetCompletedValue();

        // Release executed allocations.
        while (!mFenceValueAndPageIdPairQueue.empty())
        {
            auto [fenceValue, pageId] = mFenceValueAndPageIdPairQueue.front();
            if (fenceValue > completedFenceValue)
            {
                break;
            }

            mFenceValueAndPageIdPairQueue.pop();
            ReleaseAllocation(pageId);
        }
    }

    int UploadManager::GetAvailableCommandListIndex()
    {
        Uint64 completedFenceValue = mFence->GetCompletedValue();

        // Reuse the first command list whose previous submission has completed.
        for (int i = 0; i < MAX_COMMAND_LIST_SIZE; ++i)
        {
            if (mCopyCommandLists[i].lastFenceValue <= completedFenceValue)
            {
                return i;
            }
        }

        // All command lists are in flight. Wait for the oldest one.
        int oldestIndex = 0;
        for (int i = 1; i < MAX_COMMAND_LIST_SIZE; ++i)
        {
            if (mCopyCommandLists[i].lastFenceValue < mCopyCommandLists[oldestIndex].lastFenceValue)
            {
                oldestIndex = i;
            }
        }
        mFence->Wait(mCopyCommandLists[oldestIndex].lastFenceValue);
        return oldestIndex;
    }
} // namespace cube
