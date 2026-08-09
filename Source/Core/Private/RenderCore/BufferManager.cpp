#include "BufferManager.h"

#include "GAPI.h"
#include "GAPI_CommandList.h"
#include "GAPI_Fence.h"

namespace cube
{
    BufferManager::BufferManager(Renderer& renderer)
        : mRenderer(renderer)
    {
    }

    void BufferManager::Initialize(GAPI* gapi)
    {
        mGAPI = gapi;

        mBufferInitCommandList = mGAPI->CreateCommandList({
            .debugName = CUBE_T("BufferInitCommandList")
        });
        mBufferInitFence = mGAPI->CreateFence({
            .allowCPUAccess = true,
            .debugName = CUBE_T("BufferInitFence")
        });
    }

    void BufferManager::Shutdown()
    {
        mBufferInitFence = nullptr;
        mBufferInitCommandList = nullptr;
    }
} // namespace cube
