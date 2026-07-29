#pragma once

#include "DX12Header.h"

#include "GAPI_Fence.h"

#include "DX12Fence.h"

namespace cube
{
    class DX12Device;

    namespace gapi
    {
        class DX12Fence : public Fence
        {
        public:
            DX12Fence(const FenceCreateInfo& info, DX12Device& device);
            virtual ~DX12Fence();

            virtual void Wait(Uint64 fenceValue) override;
            virtual Uint64 GetCompletedValue() override;

            void Signal(ID3D12CommandQueue* queue, Uint64 fenceValue);

        private:
            cube::DX12Fence mFence;
        };
    } // namespace gapi
} // namespace cube
