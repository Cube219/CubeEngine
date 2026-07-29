#pragma once

#include "DX12Header.h"

#include "GAPI_Fence.h"

#include "DX12APIObject.h"
#include "DX12FenceWrapper.h"

namespace cube
{
    class DX12Device;

    namespace gapi
    {
        class DX12Fence : public Fence, public DX12APIObject
        {
        public:
            DX12Fence(const FenceCreateInfo& createInfo, DX12Device& device);
            virtual ~DX12Fence();

            virtual void Wait(Uint64 fenceValue) override;
            virtual void Signal(Uint64 fenceValue) override;

            virtual Uint64 GetCompletedValue() override;

            void SignalToQueue(ID3D12CommandQueue* queue, Uint64 fenceValue);
            void WaitOnQueue(ID3D12CommandQueue* queue, Uint64 fenceValue);

        private:
            DX12FenceWrapper mFence;
        };
    } // namespace gapi
} // namespace cube
