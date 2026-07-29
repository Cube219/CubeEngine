#include "DX12FenceWrapper.h"

#include "DX12Device.h"
#include "DX12Utility.h"

namespace cube
{
    DX12FenceWrapper::DX12FenceWrapper(DX12Device& device)
        : mDevice(device)
        , mFenceEvent(nullptr)
        , mLastSignalFenceValue(0)
    {
    }

    DX12FenceWrapper::~DX12FenceWrapper()
    {
    }

    void DX12FenceWrapper::Initialize(StringView debugName, bool allowCPUAccess)
    {
        CHECK_HR(mDevice.GetDevice()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence)));
        SET_DEBUG_NAME(mFence, debugName);

        if (allowCPUAccess)
        {
            mFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
            if (mFenceEvent == nullptr)
            {
                CHECK_HR(HRESULT_FROM_WIN32(GetLastError()));
            }
        }
        mLastSignalFenceValue = 0;
    }

    void DX12FenceWrapper::Shutdown(bool skipPendingSignal)
    {
        if (!skipPendingSignal && mFenceEvent)
        {
            Wait(mLastSignalFenceValue);
        }

        if (mFenceEvent)
        {
            CloseHandle(mFenceEvent);
        }
        mFence = nullptr;
    }

    void DX12FenceWrapper::SignalToQueue(ID3D12CommandQueue* queue, Uint64 fenceValue)
    {
        CHECK_HR(queue->Signal(mFence.Get(), fenceValue));
        mLastSignalFenceValue = fenceValue;
    }

    void DX12FenceWrapper::WaitOnQueue(ID3D12CommandQueue* queue, Uint64 fenceValue)
    {
        CHECK_HR(queue->Wait(mFence.Get(), fenceValue));
    }

    void DX12FenceWrapper::Signal(Uint64 fenceValue)
    {
        CHECK_HR(mFence->Signal(fenceValue));
    }

    void DX12FenceWrapper::Wait(Uint64 fenceValue)
    {
        Uint64 comp = mFence->GetCompletedValue();
        if (comp < fenceValue)
        {
            CHECK(mFenceEvent);
            CHECK_HR(mFence->SetEventOnCompletion(fenceValue, mFenceEvent));
            WaitForSingleObjectEx(mFenceEvent, INFINITE, FALSE);
        }
    }

    Uint64 DX12FenceWrapper::GetCompletedValue()
    {
        return mFence->GetCompletedValue();
    }
} // namespace cube
