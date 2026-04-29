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

            virtual SharedPtr<BufferSRV> CreateSRV(const BufferSRVCreateInfo& createInfo) override;
            virtual SharedPtr<BufferUAV> CreateUAV(const BufferUAVCreateInfo& createInfo) override;

            virtual void* Map() override;
            virtual void Unmap() override;

            virtual void SetDebugName(StringView debugName) override;

            ID3D12Resource* GetResource() const { return mAllocation.resource; }

        private:
            DX12Device& mDevice;

            DX12Allocation mAllocation;
            DX12UploadDesc mUploadDesc;
        };

        class DX12BufferSRV : public BufferSRV, public DX12APIObject
        {
        public:
            DX12BufferSRV(DX12Device& device, const BufferSRVCreateInfo& createInfo, SharedPtr<DX12Buffer> dx12Buffer);
            virtual ~DX12BufferSRV();

        private:
            DX12Device& mDevice;

            DX12DescriptorHandle mSRVDescriptor;
        };

        class DX12BufferUAV : public BufferUAV, public DX12APIObject
        {
        public:
            DX12BufferUAV(DX12Device& device, const BufferUAVCreateInfo& createInfo, SharedPtr<DX12Buffer> dx12Buffer);
            virtual ~DX12BufferUAV();

        private:
            DX12Device& mDevice;

            DX12DescriptorHandle mUAVDescriptor;
        };
    } // namespace gapi
} // namespace cube
