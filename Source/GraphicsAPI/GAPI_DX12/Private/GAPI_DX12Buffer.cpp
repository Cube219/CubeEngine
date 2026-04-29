#include "GAPI_DX12Buffer.h"

#include "DX12Device.h"
#include "DX12Types.h"

namespace cube
{
    namespace gapi
    {
        DX12Buffer::DX12Buffer(const BufferCreateInfo& info, DX12Device& device) :
            Buffer(info),
            mDevice(device)
        {
            CHECK(info.bufferInfo.size % info.bufferInfo.stride == 0);

            if (mInfo.type == BufferType::Constant)
            {
                CHECK_FORMAT(info.bufferInfo.stride == 1, "You should use stride 1 in constant buffer.");

                if ((mInfo.size & 255) != 0)
                {
                    Uint64 newSize = (mInfo.size + 255) & ~255;
                    CUBE_LOG(Warning, DX12, "CB size should be 256 byte aligned. Size will be changed. ({0} -> {1})", mInfo.size, newSize);
                    mInfo.size = newSize;
                }
            }

            D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;
            if (info.bufferInfo.flags.IsSet(BufferFlag::UAV))
            {
                flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
            }

            D3D12_RESOURCE_DESC desc = {
                .Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
                .Alignment = 0,
                .Width = mInfo.size,
                .Height = 1,
                .DepthOrArraySize = 1,
                .MipLevels = 1,
                .Format = DXGI_FORMAT_UNKNOWN,
                .SampleDesc = {
                    .Count = 1,
                    .Quality = 0
                },
                .Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
                .Flags = flags
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
        }

        DX12Buffer::~DX12Buffer()
        {
            mDevice.GetMemoryAllocator().Free(mAllocation);
        }

        SharedPtr<BufferSRV> DX12Buffer::CreateSRV(const BufferSRVCreateInfo& createInfo)
        {
            return std::make_shared<DX12BufferSRV>(mDevice, createInfo, std::dynamic_pointer_cast<DX12Buffer>(shared_from_this()));
        }

        SharedPtr<BufferUAV> DX12Buffer::CreateUAV(const BufferUAVCreateInfo& createInfo)
        {
            return std::make_shared<DX12BufferUAV>(mDevice, createInfo, std::dynamic_pointer_cast<DX12Buffer>(shared_from_this()));
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
                    mUploadDesc = mDevice.GetUploadManager().Allocate(ResourceType::Buffer, mInfo.size);
                    return mUploadDesc.pData;
                }
            case ResourceUsage::CPUtoGPU:
                mAllocation.Map();
                return mAllocation.pMapPtr;
            case ResourceUsage::GPUtoCPU:
                NOT_IMPLEMENTED();
                return nullptr;
            case ResourceUsage::Transient:
                NO_ENTRY_FORMAT("Cannot map transient resource.");
                return nullptr;
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
            case ResourceUsage::GPUtoCPU:
                NOT_IMPLEMENTED();
                break;
            case ResourceUsage::Transient:
                NO_ENTRY_FORMAT("Cannot unmap transient resource.");
                break;
            default:
                NOT_IMPLEMENTED();
                break;
            }
        }

        void DX12Buffer::SetDebugName(StringView debugName)
        {
            Buffer::SetDebugName(debugName);

            SET_DEBUG_NAME(mAllocation.resource, debugName);
        }

        DX12BufferSRV::DX12BufferSRV(DX12Device& device, const BufferSRVCreateInfo& createInfo, SharedPtr<DX12Buffer> dx12Buffer)
            : BufferSRV(createInfo, dx12Buffer)
            , mDevice(device)
        {
            const BufferInfo& bufferInfo = dx12Buffer->GetInfo();

            mSRVDescriptor = device.GetDescriptorManager().GetSRVHeap().Allocate();

            if (bufferInfo.type == BufferType::Constant)
            {
                const D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {
                    .BufferLocation = dx12Buffer->GetResource()->GetGPUVirtualAddress() + mFirstElement,
                    .SizeInBytes = static_cast<UINT>(mNumElements)
                };
                mDevice.GetDevice()->CreateConstantBufferView(&cbvDesc, mSRVDescriptor.cpuHandle);
            }
            else
            {
                DXGI_FORMAT format;
                if (bufferInfo.type == BufferType::Structured)
                {
                    format = DXGI_FORMAT_UNKNOWN;
                }
                else if (bufferInfo.type == BufferType::Raw)
                {
                    format = DXGI_FORMAT_R32_TYPELESS;
                }
                else
                {
                    format = GetDX12ElementFormatInfo(createInfo.typedFormat).format;
                }

                D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
                srvDesc.Format = format;
                srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
                srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
                srvDesc.Buffer.FirstElement = mFirstElement;
                srvDesc.Buffer.NumElements = static_cast<UINT>(mNumElements);
                srvDesc.Buffer.StructureByteStride = bufferInfo.type == BufferType::Structured ? bufferInfo.stride : 0;
                srvDesc.Buffer.Flags = bufferInfo.type == BufferType::Raw ? D3D12_BUFFER_SRV_FLAG_RAW : D3D12_BUFFER_SRV_FLAG_NONE;

                device.GetDevice()->CreateShaderResourceView(dx12Buffer->GetResource(), &srvDesc, mSRVDescriptor.cpuHandle);
            }

            mBindlessId = mSRVDescriptor.index;
        }

        DX12BufferSRV::~DX12BufferSRV()
        {
            mDevice.GetDescriptorManager().GetSRVHeap().Free(mSRVDescriptor);
        }

        DX12BufferUAV::DX12BufferUAV(DX12Device& device, const BufferUAVCreateInfo& createInfo, SharedPtr<DX12Buffer> dx12Buffer)
            : BufferUAV(createInfo, dx12Buffer)
            , mDevice(device)
        {
            const BufferInfo& bufferInfo = dx12Buffer->GetInfo();

            mUAVDescriptor = device.GetDescriptorManager().GetSRVHeap().Allocate();

            DXGI_FORMAT format;
            if (bufferInfo.type == BufferType::Structured)
            {
                format = DXGI_FORMAT_UNKNOWN;
            }
            else if (bufferInfo.type == BufferType::Raw)
            {
                format = DXGI_FORMAT_R32_TYPELESS;
            }
            else
            {
                format = GetDX12ElementFormatInfo(createInfo.typedFormat).format;
            }

            D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
            uavDesc.Format = format;
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
            uavDesc.Buffer.FirstElement = mFirstElement;
            uavDesc.Buffer.NumElements = static_cast<UINT>(mNumElements);
            uavDesc.Buffer.StructureByteStride = bufferInfo.type == BufferType::Structured ? bufferInfo.stride : 0;
            uavDesc.Buffer.Flags = bufferInfo.type == BufferType::Raw ? D3D12_BUFFER_UAV_FLAG_RAW : D3D12_BUFFER_UAV_FLAG_NONE;

            device.GetDevice()->CreateUnorderedAccessView(dx12Buffer->GetResource(), nullptr, &uavDesc, mUAVDescriptor.cpuHandle);
            mBindlessId = mUAVDescriptor.index;
        }

        DX12BufferUAV::~DX12BufferUAV()
        {
            mDevice.GetDescriptorManager().GetSRVHeap().Free(mUAVDescriptor);
        }
    } // namespace gapi
} // namespace cube
