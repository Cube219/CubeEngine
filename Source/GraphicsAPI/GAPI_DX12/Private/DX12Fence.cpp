#include "DX12Fence.h"

#include "DX12Device.h"
#include "DX12Utility.h"

namespace cube
{
    DX12Fence::DX12Fence(DX12Device& device) :
        mDevice(device)
    {
    }

    DX12Fence::~DX12Fence()
    {
    }

    void DX12Fence::Initialize(StringView debugName)
    {
        CHECK_HR(mDevice.GetDevice()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence)));
        SET_DEBUG_NAME(mFence, debugName);

        mFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (mFenceEvent == nullptr)
        {
            CHECK_HR(HRESULT_FROM_WIN32(GetLastError()));
        }

        mLastSignalFenceValue = 0;
    }

    void DX12Fence::Shutdown(bool skipPendingSignal)
    {
        if (!skipPendingSignal)
        {
            Wait(mLastSignalFenceValue);
        }

        CloseHandle(mFenceEvent);
        mFence = nullptr;
    }

    void DX12Fence::Signal(ID3D12CommandQueue* queue, DX12FenceValue fenceValue)
    {
        queue->Signal(mFence.Get(), fenceValue);
        mLastSignalFenceValue = fenceValue;
    }

    void DX12Fence::Wait(DX12FenceValue fenceValue)
    {
        DX12FenceValue comp = mFence->GetCompletedValue();
        if (comp < fenceValue)
        {
            CHECK_HR(mFence->SetEventOnCompletion(fenceValue, mFenceEvent));
            WaitForSingleObjectEx(mFenceEvent, INFINITE, FALSE);
        }
    }

    DX12FenceValue DX12Fence::GetCompletedValue()
    {
        return mFence->GetCompletedValue();
    }
} // namespace cube
