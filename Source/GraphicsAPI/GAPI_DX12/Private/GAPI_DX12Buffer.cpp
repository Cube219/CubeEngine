#include "GAPI_DX12Buffer.h"

#include "DX12Device.h"

namespace cube
{
    namespace gapi
    {
        DX12Buffer::DX12Buffer(const BufferCreateInfo& info, DX12Device& device) :
            Buffer(info),
            mDevice(device)
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
            switch (mUsage)
            {
            case ResourceUsage::GPUOnly: heapType = D3D12_HEAP_TYPE_DEFAULT; break;
            case ResourceUsage::CPUtoGPU: heapType = D3D12_HEAP_TYPE_UPLOAD; break;
            case ResourceUsage::GPUtoCPU: heapType = D3D12_HEAP_TYPE_READBACK; break;
            default:
                NOT_IMPLEMENTED();
                heapType = D3D12_HEAP_TYPE_UPLOAD; break;
            }
            mAllocation = device.GetMemoryAllocator().Allocate(heapType, desc);

            if (mType == BufferType::Constant)
            {
                CHECK_FORMAT(mUsage == ResourceUsage::CPUtoGPU, "Using other resource usage instaed of CPUtoGPU in constant buffer is not implemented.");

                if ((mSize & 255) != 0)
                {
                    Uint64 newSize = (mSize + 255) & ~255;
                    CUBE_LOG(LogType::Warning, DX12, "CB size should be 256 byte aligned. Size will be changed. ({} -> {})", mSize, newSize);
                    mSize = newSize;
                }

                mCBVDescriptor = device.GetDescriptorManager().GetSRVHeap().AllocateCPU();
                const D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {
                    .BufferLocation = mAllocation.allocation->GetResource()->GetGPUVirtualAddress(),
                    .SizeInBytes = (UINT)mSize
                };
                mDevice.GetDevice()->CreateConstantBufferView(&cbvDesc, mCBVDescriptor);
            }
        }

        DX12Buffer::~DX12Buffer()
        {
            if (mType == BufferType::Constant)
            {
                mDevice.GetDescriptorManager().GetSRVHeap().FreeCPU(mCBVDescriptor);
            }

            mDevice.GetMemoryAllocator().Free(mAllocation);
        }

        void* DX12Buffer::Map()
        {
            if (mUsage == ResourceUsage::GPUOnly || mUsage == ResourceUsage::GPUtoCPU)
            {
                NOT_IMPLEMENTED();
                return nullptr;
            }

            mAllocation.Map();
            return mAllocation.pMapPtr;
        }

        void DX12Buffer::Unmap()
        {
            if (mUsage == ResourceUsage::GPUOnly || mUsage == ResourceUsage::GPUtoCPU)
            {
                NOT_IMPLEMENTED();
                return;
            }

            mAllocation.Unmap();
        }
    } // namespace gapi
} // namespace cube
