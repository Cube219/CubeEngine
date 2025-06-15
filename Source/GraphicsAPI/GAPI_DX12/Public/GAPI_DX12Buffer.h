#pragma once

#include "DX12Header.h"

#include "GAPI_Buffer.h"

#include "DX12APIObject.h"
#include "DX12DescriptorManager.h"
#include "DX12MemoryAllocator.h"
#include "DX12UploadManager.h"

namespace cube
{
    class DX12Device;

    namespace gapi
    {
        class DX12Buffer : public Buffer, public DX12APIObject
        {
        public:
            DX12Buffer(const BufferCreateInfo& info, DX12Device& device);
            virtual ~DX12Buffer();

            virtual void* Map() override;
            virtual void Unmap() override;

            ID3D12Resource* GetResource() const { return mAllocation.allocation->GetResource(); }
            DX12DescriptorHandle GetCBVDescriptor() const { return mCBVDescriptor; }

        private:
            DX12Device& mDevice;

            DX12Allocation mAllocation;
            DX12UploadDesc mUploadDesc;

            DX12DescriptorHandle mCBVDescriptor;
        };
    } // namespace gapi
} // namespace cube
