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
        mLastFinishFenceValue = 0;

        mFinishFence = mGAPI->CreateFence({
            .allowCPUAccess = true,
            .debugName = CUBE_T("UploadManagerFence"),
        });

        mCopyCommandList = mGAPI->CreateCommandList({
            .type = gapi::CommandListType::Copy,
            .debugName = CUBE_T("UploadManagerCopyCommandList")
        });
    }

    void UploadManager::Shutdown()
    {
        mFinishFence->Wait(mLastFinishFenceValue);
        UpdateStates();
        mFinishFence = nullptr;

        mCopyCommandList = nullptr;

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

    UploadDesc UploadManager::Allocate(SharedPtr<gapi::Buffer> dstBuffer, bool directIfPossible)
    {
        CHECK(dstBuffer->GetUsage() == gapi::ResourceUsage::GPUOnly);

        if (directIfPossible && mGAPI->IsDirectMapSupported(gapi::ResourceType::Buffer))
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

    UploadDesc UploadManager::Allocate(SharedPtr<gapi::Texture> dstTexture, bool directIfPossible)
    {
        CHECK(dstTexture->GetUsage() == gapi::ResourceUsage::GPUOnly);

        if (directIfPossible && mGAPI->IsDirectMapSupported(gapi::ResourceType::Texture))
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

    Uint64 UploadManager::SubmitToCopyQueue(UploadDesc& desc)
    {
        if (IsCopyCommandListNeeded(desc))
        {
            mCopyCommandList->Reset();
            mCopyCommandList->Begin();

            Uint64 finishSignalValue = Submit(desc, mCopyCommandList);
            CHECK(finishSignalValue > 0);

            mCopyCommandList->End();

            mCopyCommandList->Submit();
            if (!desc.IsDirectWrite())
            {
                mFenceValueAndPageIdPairQueue.push({ finishSignalValue, desc.pageId });
            }

            return finishSignalValue;
        }
        else
        {
            Submit(desc, nullptr);

            return 0;
        }
    }

    Uint64 UploadManager::Submit(UploadDesc& desc, SharedPtr<gapi::CommandList> commandList)
    {
        UpdateStates();

        if (desc.IsDirectWrite())
        {
            Uint64 finishFenceValue = 0;

            if (desc.dstBuffer)
            {
                desc.dstBuffer->Unmap();
            }
            else if (desc.dstTexture)
            {
                desc.dstTexture->Unmap();

                commandList->OptimizeTextureContentsForGPUAccess(desc.dstTexture);

                mLastFinishFenceValue++;
                finishFenceValue = mLastFinishFenceValue;
                commandList->SignalToFence(mFinishFence, finishFenceValue);
            }

            desc.pData = nullptr;
            desc.dstBuffer = nullptr;
            desc.dstTexture = nullptr;

            return finishFenceValue;
        }

        auto pageIt = mPages.find(desc.pageId);
        CHECK(pageIt != mPages.end());
        Page& page = pageIt->second;

        if (desc.dstBuffer)
        {
            commandList->CopyBuffer(page.stagingBuffer, desc.offsetInPage, desc.dstBuffer, 0, desc.size);
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
            commandList->SetResourceBarrier(barrier);

            commandList->CopyBufferToTexture(page.stagingBuffer, desc.offsetInPage, desc.dstTexture);

            commandList->OptimizeTextureContentsForGPUAccess(desc.dstTexture);
        }

        desc.pData = nullptr;
        desc.dstBuffer = nullptr;
        desc.dstTexture = nullptr;

        mLastFinishFenceValue++;
        commandList->SignalToFence(mFinishFence, mLastFinishFenceValue);

        return mLastFinishFenceValue;
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

            // Just add to release queue to identify the uploading was completed in UpdateStates.
            // The page can be used in other in-flight upload descs, so the release
            // must be deferred until all previously submitted uploads complete.
            // (See ReleaseAllocation().)
            mFenceValueAndPageIdPairQueue.push({ 0, desc.pageId });
        }

        desc.pData = nullptr;
        desc.dstBuffer = nullptr;
        desc.dstTexture = nullptr;
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
        Uint64 completedFenceValue = mFinishFence->GetCompletedValue();

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

    bool UploadManager::IsCopyCommandListNeeded(const UploadDesc& desc) const
    {
        return !desc.IsDirectWrite() || (desc.dstTexture && mGAPI->IsNeededToOptimizeTextureContentsUsingCommandList());
    }
} // namespace cube
