#include "DX12QueueManager.h"

#include "DX12Device.h"
#include "DX12Utility.h"

namespace cube
{
    DX12QueueManager::DX12QueueManager(DX12Device& device) :
        mDevice(device)
    {
    }

    void DX12QueueManager::Initialize()
    {
        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        CHECK_HR(mDevice.GetDevice()->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&mMainQueue)));
        SET_DEBUG_NAME(mMainQueue, CUBE_T("MainCommandQueue"));

        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
        CHECK_HR(mDevice.GetDevice()->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&mCopyQueue)));
        SET_DEBUG_NAME(mMainQueue, CUBE_T("CopyCommandQueue"));
    }

    void DX12QueueManager::Shutdown()
    {
        mCopyQueue = nullptr;
        mMainQueue = nullptr;
    }
} // namespace cube
