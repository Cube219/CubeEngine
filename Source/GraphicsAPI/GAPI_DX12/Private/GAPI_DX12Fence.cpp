#include "GAPI_DX12Fence.h"

#include "DX12Device.h"

namespace cube
{
    namespace gapi
    {
        DX12Fence::DX12Fence(const FenceCreateInfo& createInfo, DX12Device& device)
            : Fence(createInfo)
            , mFence(device)
        {
            mFence.Initialize(createInfo.debugName, createInfo.allowCPUAccess);
        }

        DX12Fence::~DX12Fence()
        {
            mFence.Shutdown();
        }

        void DX12Fence::Wait(Uint64 fenceValue)
        {
            CHECK(mAllowCPUAccess);

            mFence.Wait(fenceValue);
        }

        void DX12Fence::Signal(Uint64 fenceValue)
        {
            CHECK(mAllowCPUAccess);

            mFence.Signal(fenceValue);
        }

        Uint64 DX12Fence::GetCompletedValue()
        {
            CHECK(mAllowCPUAccess);

            return mFence.GetCompletedValue();
        }

        void DX12Fence::SignalToQueue(ID3D12CommandQueue* queue, Uint64 fenceValue)
        {
            mFence.SignalToQueue(queue, fenceValue);
        }

        void DX12Fence::WaitOnQueue(ID3D12CommandQueue* queue, Uint64 fenceValue)
        {
            mFence.WaitOnQueue(queue, fenceValue);
        }
    } // namespace gapi
} // namespace cube
