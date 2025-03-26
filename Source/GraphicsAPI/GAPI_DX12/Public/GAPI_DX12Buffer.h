#pragma once

#include "DX12Header.h"

#include "GAPI_Buffer.h"

#include "DX12MemoryAllocator.h"

namespace cube
{
    class DX12Device;

    namespace gapi
    {
        class DX12Buffer : public Buffer
        {
        public:
            DX12Buffer(const BufferCreateInfo& info, DX12Device& device);
            virtual ~DX12Buffer();

            virtual void* Map() override;
            virtual void Unmap() override;

            ID3D12Resource* GetResource() const { return mAllocation.allocation->GetResource(); }
            D3D12_CPU_DESCRIPTOR_HANDLE GetCBVDescriptor() const { return mCBVDescriptor; }

        private:
            DX12Device& mDevice;

            DX12Allocation mAllocation;

            D3D12_CPU_DESCRIPTOR_HANDLE mCBVDescriptor;
        };
    } // namespace gapi
} // namespace cube
