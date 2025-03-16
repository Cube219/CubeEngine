#include "GAPI_DX12Buffer.h"

#include "DX12Device.h"

namespace cube
{
    namespace gapi
    {
        DX12Buffer::DX12Buffer(const BufferCreateInfo& info, DX12Device& device) :
            mDevice(device),
            mUsage(info.usage),
            mSize(info.size)
        {
            D3D12_RESOURCE_DESC desc = {
                .Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
                .Alignment = 0,
                .Width = info.size,
                .Height = 1,
                .DepthOrArraySize = 1,
                .MipLevels = 1,
                .Format = DXGI_FORMAT_UNKNOWN,
                .SampleDesc = {
                    .Count = 1,
                    .Quality = 0 },
                .Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
                .Flags = D3D12_RESOURCE_FLAG_NONE
            };

            D3D12_HEAP_TYPE heapType;
            switch (info.usage)
            {
            case ResourceUsage::GPUOnly: heapType = D3D12_HEAP_TYPE_DEFAULT; break;
            case ResourceUsage::CPUtoGPU: heapType = D3D12_HEAP_TYPE_UPLOAD; break;
            case ResourceUsage::GPUtoCPU: heapType = D3D12_HEAP_TYPE_READBACK; break;
            default: heapType = D3D12_HEAP_TYPE_UPLOAD; break;
            }
            mAllocation = device.GetMemoryAllocator().Allocate(heapType, desc);
        }

        DX12Buffer::~DX12Buffer()
        {
            mDevice.GetMemoryAllocator().Free(mAllocation);
        }

        void* DX12Buffer::Map()
        {
            if (mUsage == ResourceUsage::GPUOnly)
            {
                NOT_IMPLEMENTED();
                return nullptr;
            }

            mAllocation.Map();
            return mAllocation.pMapPtr;
        }

        void DX12Buffer::Unmap()
        {
            if (mUsage == ResourceUsage::GPUOnly)
            {
                NOT_IMPLEMENTED();
            }

            mAllocation.Unmap();
        }
    } // namespace gapi
} // namespace cube
