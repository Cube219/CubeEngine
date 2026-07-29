#include "GAPI_DX12Fence.h"

#include "DX12Device.h"

namespace cube
{
    namespace gapi
    {
        DX12Fence::DX12Fence(const FenceCreateInfo& info, DX12Device& device)
            : mFence(device)
        {
            mFence.Initialize(info.debugName);
        }

        DX12Fence::~DX12Fence()
        {
            mFence.Shutdown();
        }

        void DX12Fence::Wait(Uint64 fenceValue)
        {
            mFence.Wait(fenceValue);
        }

        Uint64 DX12Fence::GetCompletedValue()
        {
            return mFence.GetCompletedValue();
        }

        void DX12Fence::Signal(ID3D12CommandQueue* queue, Uint64 fenceValue)
        {
            mFence.Signal(queue, fenceValue);
        }
    } // namespace gapi
} // namespace cube
