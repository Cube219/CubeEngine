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

            void CopyToReadbackBuffer(ID3D12GraphicsCommandList* commandList);

        private:
            DX12Device& mDevice;

            DX12Allocation mAllocation;
            DX12UploadDesc mUploadDesc;

            DX12Allocation mReadbackAllocation;
        };

        class DX12BufferSRV : public BufferSRV, public DX12APIObject
        {
        public:
            DX12BufferSRV(DX12Device& device, const BufferSRVCreateInfo& createInfo, SharedPtr<DX12Buffer> dx12Buffer);
            virtual ~DX12BufferSRV();

            D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress() const { return mGPUAddress; }

        private:
            DX12Device& mDevice;

            DX12DescriptorHandle mSRVDescriptor;
            D3D12_GPU_VIRTUAL_ADDRESS mGPUAddress;
        };

        class DX12BufferUAV : public BufferUAV, public DX12APIObject
        {
        public:
            DX12BufferUAV(DX12Device& device, const BufferUAVCreateInfo& createInfo, SharedPtr<DX12Buffer> dx12Buffer);
            virtual ~DX12BufferUAV();

            D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress() const { return mGPUAddress; }

        private:
            DX12Device& mDevice;

            DX12DescriptorHandle mUAVDescriptor;
            D3D12_GPU_VIRTUAL_ADDRESS mGPUAddress;
        };
    } // namespace gapi
} // namespace cube
