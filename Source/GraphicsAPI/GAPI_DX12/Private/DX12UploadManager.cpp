#include "DX12UploadManager.h"

#include "Allocator/AllocatorUtility.h"
#include "DX12APIObject.h"
#include "DX12Device.h"

namespace cube
{
    DX12UploadManager::DX12UploadManager(DX12Device& device) :
        mDevice(device),
        mFence(device)
    {
        mLastFenceValue = 0;
    }

    void DX12UploadManager::Initialize(Uint64 minPageSize)
    {
        mMinPageSize = minPageSize;
        mPageNextId = 0;
        mFence.Initialize(CUBE_T("UploadManagerFence"));

        int index = 0;
        for (CommandListAllocator& allocator : mCommandListAllocators)
        {
            CHECK_HR(mDevice.GetDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY, IID_PPV_ARGS(&allocator.allocator)));
            SET_DEBUG_NAME_FORMAT(allocator.allocator, "CommandList allocator used in upload manager {0}", index);
            allocator.lastFenceValue = 0;

            index++;
        }
        mCurrentAllocatorIndex = 0;
    }

    void DX12UploadManager::Shutdown()
    {
        mFence.Wait(mLastFenceValue);
        // For clear bound objects
        UpdateStates();
        mFence.Shutdown();

        for (CommandListAllocator& allocator : mCommandListAllocators)
        {
            allocator.allocator = nullptr;
        }

        for (auto& [_, page] : mPages)
        {
            CHECK(page.refCount == 0);
            mDevice.GetMemoryAllocator().Free(page.allocation);
        }
        mPages.clear();
    }
    
    DX12UploadDesc DX12UploadManager::Allocate(gapi::ResourceType type, Uint64 size, Uint64 alignment)
    {
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
        Uint64 ptr = (Uint64)page.allocation.pMapPtr + page.offset;
        Uint64 alignPtr = Align(ptr, alignment);
        Uint64 alignGap = alignPtr - ptr;

        DX12UploadDesc res = {
            .type = type,
            .pData = (Byte*)page.allocation.pMapPtr + page.offset + alignGap,
            .size = size,
            .dstResource = nullptr,
            .dstAPIObject = nullptr,
            .dstTextureLayout = {},
            .pageId = pageId,
            .offsetInPage = page.offset,
        };

        page.offset += size + alignGap;
        page.refCount++;
        mPageRemainSizeMap.insert({ page.size - page.offset, pageId });

        return res;
    }

    DX12FenceValue DX12UploadManager::Submit(DX12UploadDesc& desc, bool waitForCompletion)
    {
        UpdateStates();

        CommandListAllocator& allocator = mCommandListAllocators[mCurrentAllocatorIndex];
        auto pageIt = mPages.find(desc.pageId);
        CHECK(pageIt != mPages.end());
        Page& page = pageIt->second;

        ComPtr<ID3D12GraphicsCommandList> commandList;
        CHECK_HR(mDevice.GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COPY, allocator.allocator.Get(), nullptr, IID_PPV_ARGS(&commandList)));

        if (desc.type == gapi::ResourceType::Buffer)
        {
            commandList->CopyBufferRegion(desc.dstResource, 0, page.allocation.allocation->GetResource(), desc.offsetInPage, desc.size);
        }
        else if (desc.type == gapi::ResourceType::Texture)
        {
            D3D12_PLACED_SUBRESOURCE_FOOTPRINT srcFootprint = desc.dstTextureLayout;
            srcFootprint.Offset = desc.offsetInPage;
            D3D12_TEXTURE_COPY_LOCATION src = {
                .pResource = page.allocation.allocation->GetResource(),
                .Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT,
                .PlacedFootprint = srcFootprint
            };
            D3D12_TEXTURE_COPY_LOCATION dst = {
                .pResource = desc.dstResource,
                .Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX,
                .SubresourceIndex = 0
            };
            commandList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
        }
        else
        {
            NOT_IMPLEMENTED();
        }
        allocator.boundObjectsInCommand.push_back(desc.dstAPIObject->shared_from_this());
        commandList->Close();

        ID3D12CommandList* cmdLists[] = { commandList.Get() };
        ID3D12CommandQueue* copyQueue = mDevice.GetQueueManager().GetCopyQueue();
        copyQueue->ExecuteCommandLists(1, cmdLists);

        mLastFenceValue++;
        mFence.Signal(copyQueue, mLastFenceValue);
        allocator.lastFenceValue = mLastFenceValue;
        mFenceValueAndPageIdPairQueue.push({ mLastFenceValue, desc.pageId });
        
        desc.pData = nullptr;
        desc.dstAPIObject = nullptr;

        if (waitForCompletion)
        {
            mFence.Wait(mLastFenceValue);
        }

        return mLastFenceValue;
    }

    bool DX12UploadManager::IsUploadFinished(DX12FenceValue submitFenceValue)
    {
        return submitFenceValue <= mFence.GetCompletedValue();
    }

    int DX12UploadManager::AllocateNewPage(Uint64 size)
    {
        size = std::max(size, mMinPageSize);

        D3D12_RESOURCE_DESC desc = {
            .Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
            .Alignment = 0,
            .Width = size,
            .Height = 1,
            .DepthOrArraySize = 1,
            .MipLevels = 1,
            .Format = DXGI_FORMAT_UNKNOWN,
            .SampleDesc = {
                .Count = 1,
                .Quality = 0
            },
            .Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
            .Flags = D3D12_RESOURCE_FLAG_NONE
        };
        Page newPage = {
            .refCount = 0,
            .allocation = mDevice.GetMemoryAllocator().Allocate(D3D12_HEAP_TYPE_UPLOAD, desc),
            .size = size,
            .offset = 0
        };
        newPage.allocation.Map();
        mPages.insert({ mPageNextId, newPage });
        mPageNextId++;

        mPageRemainSizeMap.insert({ size, mPageNextId - 1 });

        return mPageNextId - 1;
    }

    void DX12UploadManager::ReleaseAllocation(int pageId)
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

    void DX12UploadManager::UpdateStates()
    {
        DX12FenceValue completedFenceValue = mFence.GetCompletedValue();

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

        // Reset the command list allocator if it's all command lists are executed.
        for (CommandListAllocator& allocator : mCommandListAllocators)
        {
            if (!allocator.boundObjectsInCommand.empty() && allocator.lastFenceValue >= completedFenceValue)
            {
                CHECK_HR(allocator.allocator->Reset());
                allocator.boundObjectsInCommand.clear();
            }
        }

        // Change the allocator if empty allocator is existed.
        int emptyAllocatorIndex = -1;
        for (int i = 0; i < MAX_ALLOCATOR_SIZE; ++i)
        {
            if (mCommandListAllocators[i].lastFenceValue >= completedFenceValue)
            {
                emptyAllocatorIndex = i;
                break;
            }
        }
        if (emptyAllocatorIndex != -1)
        {
            mCurrentAllocatorIndex = emptyAllocatorIndex;
        }
    }
} // namespace cube
