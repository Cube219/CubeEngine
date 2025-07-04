#include "GAPI_DX12Texture.h"

#include "DX12Device.h"
#include "DX12Types.h"

namespace cube
{
    namespace gapi
    {
        DX12Texture::DX12Texture(const TextureCreateInfo& info, DX12Device& device) :
            Texture(info),
            mDevice(device)
        {
            D3D12_RESOURCE_DIMENSION dimension;
            switch (info.type)
            {
            case TextureType::Texture1D:
            case TextureType::Texture1DArray:
                dimension = D3D12_RESOURCE_DIMENSION_TEXTURE1D;
                break;
            case TextureType::Texture2D:
            case TextureType::Texture2DArray:
            case TextureType::TextureCube:
            case TextureType::TextureCubeArray:
                dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
                break;
            case TextureType::Texture3D:
                dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
                break;
            default:
                NOT_IMPLEMENTED();
                dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
            }

            D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;
            if (info.flags.IsSet(TextureFlag::RenderTarget))
            {
                flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
            }
            if (info.flags.IsSet(TextureFlag::UAV))
            {
                flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
            }

            D3D12_RESOURCE_DESC desc = {
                .Dimension = dimension,
                .Alignment = 0,
                .Width = info.width,
                .Height = info.height,
                .DepthOrArraySize = static_cast<UINT16>(std::max(info.depth, info.arraySize)),
                .MipLevels = static_cast<UINT16>(info.mipLevels),
                .Format = GetDX12ElementFormatInfo(info.format).format,
                .SampleDesc = {
                    .Count = 1,
                    .Quality = 0 },
                .Layout = (mUsage == ResourceUsage::GPUOnly) ? D3D12_TEXTURE_LAYOUT_UNKNOWN : D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
                .Flags = flags
            };
            device.GetDevice()->GetCopyableFootprints(&desc, 0, 1, 0, &mLayout, nullptr, nullptr, &mTotalSize);
            mRowPitch = mLayout.Footprint.RowPitch;

            D3D12_HEAP_TYPE heapType;
            switch (mUsage)
            {
            case ResourceUsage::GPUOnly: heapType = D3D12_HEAP_TYPE_DEFAULT; break;
            case ResourceUsage::CPUtoGPU: heapType = D3D12_HEAP_TYPE_UPLOAD; break;
            case ResourceUsage::GPUtoCPU: heapType = D3D12_HEAP_TYPE_READBACK; break;
            default:
                NOT_IMPLEMENTED();
                heapType = D3D12_HEAP_TYPE_UPLOAD;
                break;
            }
            mAllocation = device.GetMemoryAllocator().Allocate(heapType, desc);
            SET_DEBUG_NAME(mAllocation.allocation->GetResource(), info.debugName);
        }

        DX12Texture::~DX12Texture()
        {
            mDevice.GetMemoryAllocator().Free(mAllocation);
        }

        void* DX12Texture::Map()
        {
            switch (mUsage)
            {
            case ResourceUsage::GPUOnly:
                mUploadDesc = mDevice.GetUploadManager().Allocate(ResourceType::Texture, mTotalSize, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);
                return mUploadDesc.pData;
            case ResourceUsage::CPUtoGPU:
                mAllocation.Map();
                return mAllocation.pMapPtr;
            case ResourceUsage::GPUtoCPU:
            default:
                NOT_IMPLEMENTED();
                return nullptr;
            }
        }

        void DX12Texture::Unmap()
        {
            switch (mUsage)
            {
            case ResourceUsage::GPUOnly:
                mUploadDesc.type = ResourceType::Texture;
                mUploadDesc.dstResource = mAllocation.allocation->GetResource();
                mUploadDesc.dstAPIObject = this;
                mUploadDesc.dstTextureLayout = mLayout;

                mDevice.GetUploadManager().Submit(mUploadDesc, true);
                break;
            case ResourceUsage::CPUtoGPU:
                mAllocation.Unmap();
                break;
            case ResourceUsage::GPUtoCPU:
            default:
                NOT_IMPLEMENTED();
                break;
            }
        }

        SharedPtr<TextureSRV> DX12Texture::CreateSRV(const TextureSRVCreateInfo& createInfo)
        {
            return std::make_shared<DX12TextureSRV>(createInfo, std::dynamic_pointer_cast<DX12Texture>(shared_from_this()),mDevice);
        }

        SharedPtr<TextureUAV> DX12Texture::CreateUAV(const TextureUAVCreateInfo& createInfo)
        {
            return std::make_shared<DX12TextureUAV>(createInfo, std::dynamic_pointer_cast<DX12Texture>(shared_from_this()), mDevice);
        }

        DX12TextureSRV::DX12TextureSRV(const TextureSRVCreateInfo& createInfo, SharedPtr<Texture> texture, DX12Device& device) :
            TextureSRV(createInfo, texture),
            mDevice(device)
        {
            SharedPtr<DX12Texture> dx12Texture = std::dynamic_pointer_cast<DX12Texture>(texture);
            CHECK(dx12Texture);

            mSRVDescriptor = device.GetDescriptorManager().GetSRVHeap().AllocateCPU();
            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Format = GetDX12ElementFormatInfo(dx12Texture->GetFormat()).format;
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            switch (dx12Texture->GetType())
            {
            case TextureType::Texture1D:
                srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
                srvDesc.Texture1D.MostDetailedMip = createInfo.firstMipLevel;
                srvDesc.Texture1D.MipLevels = createInfo.mipLevels;
                break;
            case TextureType::Texture1DArray:
                srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
                srvDesc.Texture1DArray.MostDetailedMip = createInfo.firstMipLevel;
                srvDesc.Texture1DArray.MipLevels = createInfo.mipLevels;
                srvDesc.Texture1DArray.FirstArraySlice = createInfo.firstArrayIndex;
                srvDesc.Texture1DArray.ArraySize = createInfo.arraySize;
                break;
            case TextureType::Texture2D:
                srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
                srvDesc.Texture2D.MostDetailedMip = createInfo.firstMipLevel;
                srvDesc.Texture2D.MipLevels = createInfo.mipLevels;
                break;
            case TextureType::Texture2DArray:
                srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
                srvDesc.Texture2DArray.MostDetailedMip = createInfo.firstMipLevel;
                srvDesc.Texture2DArray.MipLevels = createInfo.mipLevels;
                srvDesc.Texture2DArray.FirstArraySlice = createInfo.firstArrayIndex;
                srvDesc.Texture2DArray.ArraySize = createInfo.arraySize;
                break;
            case TextureType::Texture3D:
                srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
                srvDesc.Texture3D.MostDetailedMip = createInfo.firstMipLevel;
                srvDesc.Texture3D.MipLevels = createInfo.mipLevels;
                break;
            case TextureType::TextureCube:
                srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
                srvDesc.TextureCube.MostDetailedMip = createInfo.firstMipLevel;
                srvDesc.TextureCube.MipLevels = createInfo.mipLevels;
                break;
            case TextureType::TextureCubeArray:
                srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
                srvDesc.TextureCubeArray.MostDetailedMip = createInfo.firstMipLevel;
                srvDesc.TextureCubeArray.MipLevels = createInfo.mipLevels;
                srvDesc.TextureCubeArray.First2DArrayFace = createInfo.firstArrayIndex;
                srvDesc.TextureCubeArray.NumCubes = createInfo.arraySize;
                break;
            default:
                NOT_IMPLEMENTED();
            }

            device.GetDevice()->CreateShaderResourceView(dx12Texture->GetResource(), &srvDesc, mSRVDescriptor.handle);
            mBindlessIndex = mSRVDescriptor.index;
        }

        DX12TextureSRV::~DX12TextureSRV()
        {
            mDevice.GetDescriptorManager().GetSRVHeap().FreeCPU(mSRVDescriptor);
        }

        DX12TextureUAV::DX12TextureUAV(const TextureUAVCreateInfo& createInfo, SharedPtr<Texture> texture, DX12Device& device) :
            TextureUAV(createInfo, texture),
            mDevice(device)
        {
            SharedPtr<DX12Texture> dx12Texture = std::dynamic_pointer_cast<DX12Texture>(texture);
            CHECK(dx12Texture);

            mUAVDescriptor = device.GetDescriptorManager().GetSRVHeap().AllocateCPU();
            D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
            uavDesc.Format = GetDX12ElementFormatInfo(dx12Texture->GetFormat()).format;
            switch (dx12Texture->GetType())
            {
            case TextureType::Texture1D:
                uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
                uavDesc.Texture1D.MipSlice = createInfo.mipLevel;
                break;
            case TextureType::Texture1DArray:
                uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
                uavDesc.Texture1DArray.MipSlice = createInfo.mipLevel;
                uavDesc.Texture1DArray.FirstArraySlice = createInfo.firstArrayIndex;
                uavDesc.Texture1DArray.ArraySize = createInfo.arraySize;
                break;
            case TextureType::Texture2D:
                uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
                uavDesc.Texture2D.MipSlice = createInfo.mipLevel;
                break;
            case TextureType::Texture2DArray:
                uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
                uavDesc.Texture2DArray.MipSlice = createInfo.mipLevel;
                uavDesc.Texture2DArray.FirstArraySlice = createInfo.firstArrayIndex;
                uavDesc.Texture2DArray.ArraySize = createInfo.arraySize;
                break;
            case TextureType::Texture3D:
                uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
                uavDesc.Texture3D.MipSlice = createInfo.mipLevel;
                uavDesc.Texture3D.FirstWSlice = createInfo.firstDepthIndex;
                uavDesc.Texture3D.WSize = createInfo.DepthSize;
                break;
            case TextureType::TextureCube: // TODO
            case TextureType::TextureCubeArray:
            default:
                NOT_IMPLEMENTED();
            }

            device.GetDevice()->CreateUnorderedAccessView(dx12Texture->GetResource(), nullptr, &uavDesc, mUAVDescriptor.handle);
            mBindlessIndex = mUAVDescriptor.index;
        }

        DX12TextureUAV::~DX12TextureUAV()
        {
            mDevice.GetDescriptorManager().GetSRVHeap().FreeCPU(mUAVDescriptor);
        }
    } // namespace gapi
} // namespace cube
