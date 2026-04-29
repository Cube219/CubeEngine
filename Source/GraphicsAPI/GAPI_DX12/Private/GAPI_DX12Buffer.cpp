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
            if (mType == BufferType::Constant)
            {
                if ((mSize & 255) != 0)
                {
                    Uint64 newSize = (mSize + 255) & ~255;
                    CUBE_LOG(Warning, DX12, "CB size should be 256 byte aligned. Size will be changed. ({0} -> {1})", mSize, newSize);
                    mSize = newSize;
                }
            }

            D3D12_RESOURCE_DESC desc = {
                .Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
                .Alignment = 0,
                .Width = mSize,
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
            case ResourceUsage::GPUOnly:
                heapType = device.IsGPUUploadHeapSupported()
                    ? D3D12_HEAP_TYPE_GPU_UPLOAD
                    : D3D12_HEAP_TYPE_DEFAULT;
                break;
            case ResourceUsage::CPUtoGPU: heapType = D3D12_HEAP_TYPE_UPLOAD; break;
            case ResourceUsage::GPUtoCPU: heapType = D3D12_HEAP_TYPE_READBACK; break;
            case ResourceUsage::Transient: heapType = D3D12_HEAP_TYPE_DEFAULT; break;
            default:
                NOT_IMPLEMENTED();
                heapType = D3D12_HEAP_TYPE_UPLOAD; break;
            }
            const bool isTransient = (mUsage == ResourceUsage::Transient);
            mAllocation = device.GetMemoryAllocator().Allocate(heapType, desc, isTransient);
            SET_DEBUG_NAME(mAllocation.resource, info.debugName);

            if (mType == BufferType::Constant)
            {
                CHECK_FORMAT(mUsage == ResourceUsage::CPUtoGPU, "Using other resource usage instaed of CPUtoGPU in constant buffer is not implemented.");

                mCBVDescriptor = device.GetDescriptorManager().GetSRVHeap().Allocate();
                const D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {
                    .BufferLocation = mAllocation.resource->GetGPUVirtualAddress(),
                    .SizeInBytes = (UINT)mSize
                };
                mDevice.GetDevice()->CreateConstantBufferView(&cbvDesc, mCBVDescriptor.cpuHandle);
            }
        }

        DX12Buffer::~DX12Buffer()
        {
            if (mType == BufferType::Constant)
            {
                mDevice.GetDescriptorManager().GetSRVHeap().Free(mCBVDescriptor);
            }

            mDevice.GetMemoryAllocator().Free(mAllocation);
        }

        void* DX12Buffer::Map()
        {
            switch (mUsage)
            {
            case ResourceUsage::GPUOnly:
                if (mAllocation.heapType == D3D12_HEAP_TYPE_GPU_UPLOAD)
                {
                    mAllocation.Map();
                    return mAllocation.pMapPtr;
                }
                else
                {
                    // TODO: Use alignment?
                    mUploadDesc = mDevice.GetUploadManager().Allocate(ResourceType::Buffer, mSize);
                    return mUploadDesc.pData;
                }
            case ResourceUsage::CPUtoGPU:
                mAllocation.Map();
                return mAllocation.pMapPtr;
            case ResourceUsage::Transient:
                NO_ENTRY_FORMAT("Cannot map transient resource.");
                return nullptr;
            case ResourceUsage::GPUtoCPU:
            default:
                NOT_IMPLEMENTED();
                return nullptr;
            }
        }

        void DX12Buffer::Unmap()
        {
            switch (mUsage)
            {
            case ResourceUsage::GPUOnly:
                if (mAllocation.heapType == D3D12_HEAP_TYPE_GPU_UPLOAD)
                {
                    mAllocation.Unmap();
                }
                else
                {
                    mUploadDesc.type = ResourceType::Buffer;
                    mUploadDesc.dstResource = mAllocation.resource;
                    mUploadDesc.dstAPIObject = this;

                    mDevice.GetUploadManager().Submit(mUploadDesc, true);
                }
                break;
            case ResourceUsage::CPUtoGPU:
                mAllocation.Unmap();
                break;
            case ResourceUsage::Transient:
                NO_ENTRY_FORMAT("Cannot unmap transient resource.");
                break;
            case ResourceUsage::GPUtoCPU:
            default:
                NOT_IMPLEMENTED();
                break;
            }
        }

        void DX12Buffer::SetDebugName(StringView debugName)
        {
            SET_DEBUG_NAME(mAllocation.resource, debugName);
        }
    } // namespace gapi
} // namespace cube
