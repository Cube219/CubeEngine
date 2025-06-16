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
                dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
            }

            D3D12_RESOURCE_DESC desc = {
                .Dimension = dimension,
                .Alignment = 0,
                .Width = info.width,
                .Height = info.height,
                .DepthOrArraySize = static_cast<UINT16>(std::max(info.depth, info.arraySize)),
                .MipLevels = 1,
                .Format = GetDX12ElementFormatInfo(info.format).format,
                .SampleDesc = {
                    .Count = 1,
                    .Quality = 0 },
                .Layout = (mUsage == ResourceUsage::GPUOnly) ? D3D12_TEXTURE_LAYOUT_UNKNOWN : D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
                .Flags = D3D12_RESOURCE_FLAG_NONE
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

            // TODO: Add SRV/UAV/RTV flags
            mSRVDescriptor = device.GetDescriptorManager().GetSRVHeap().AllocateCPU();
            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Format = GetDX12ElementFormatInfo(info.format).format;
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            switch (info.type)
            {
            case TextureType::Texture1D:
                srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
                srvDesc.Texture1D.MipLevels = 1;
                break;
            case TextureType::Texture1DArray:
                srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
                srvDesc.Texture1DArray.MipLevels = 1;
                srvDesc.Texture1DArray.FirstArraySlice = 0;
                srvDesc.Texture1DArray.ArraySize = info.arraySize;
                break;
            case TextureType::Texture2D:
                srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
                srvDesc.Texture2D.MipLevels = 1;
                break;
            case TextureType::Texture2DArray:
                srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
                srvDesc.Texture2DArray.MipLevels = 1;
                srvDesc.Texture2DArray.FirstArraySlice = 0;
                srvDesc.Texture2DArray.ArraySize = info.arraySize;
                break;
            case TextureType::Texture3D:
                srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
                srvDesc.Texture3D.MipLevels = 1;
                break;
            case TextureType::TextureCube:
                srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
                srvDesc.TextureCube.MipLevels = 1;
                break;
            case TextureType::TextureCubeArray:
                srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
                srvDesc.TextureCubeArray.MipLevels = 1;
                srvDesc.TextureCubeArray.First2DArrayFace = 0;
                srvDesc.TextureCubeArray.NumCubes = info.arraySize;
                break;
            default:
                NOT_IMPLEMENTED();
            }

            device.GetDevice()->CreateShaderResourceView(mAllocation.allocation->GetResource(), &srvDesc, mSRVDescriptor.handle);
            mBindlessIndex = mSRVDescriptor.index;
        }

        DX12Texture::~DX12Texture()
        {
            mDevice.GetDescriptorManager().GetSRVHeap().FreeCPU(mSRVDescriptor);

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
    } // namespace gapi
} // namespace cube
