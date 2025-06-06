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
                .Alignment = D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT,
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
    } // namespace gapi
} // namespace cube
