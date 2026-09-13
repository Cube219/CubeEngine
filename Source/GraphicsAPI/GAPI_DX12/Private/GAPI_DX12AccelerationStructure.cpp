#include "GAPI_DX12AccelerationStructure.h"

#include "DX12Device.h"
#include "DX12Types.h"
#include "GAPI_DX12Buffer.h"

namespace cube
{
    namespace gapi
    {
        DX12BLAS::DX12BLAS(const BLASCreateInfo& createInfo, DX12Device& device)
            : BLAS(createInfo)
            , mDevice(device)
        {
            CHECK(mDevice.IsHWRTSupported());

            DX12Buffer* dx12VertexBuffer = dynamic_cast<DX12Buffer*>(createInfo.vertexBuffer.get());
            CHECK(dx12VertexBuffer);
            D3D12_GPU_VIRTUAL_ADDRESS vertexBufferGPUAddress = dx12VertexBuffer->GetGPUAddress();
            DX12ElementFormatInfo vertexFormatInfo = GetDX12ElementFormatInfo(createInfo.vertexFormat);
            CHECK(vertexFormatInfo.bytes <= createInfo.vertexStride);

            DX12Buffer* dx12IndexBuffer = dynamic_cast<DX12Buffer*>(createInfo.indexBuffer.get());
            CHECK(dx12IndexBuffer);
            D3D12_GPU_VIRTUAL_ADDRESS indexBufferGPUAddress = dx12IndexBuffer->GetGPUAddress();
            DXGI_FORMAT indexFormat = dx12IndexBuffer->GetIndexFormat();
            const Uint64 indexStride = dx12IndexBuffer->GetStride();

            mGeometryDescs.reserve(createInfo.geometryInfos.size());

            D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc;
            geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
            geometryDesc.Flags = createInfo.isOpaque ? D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE : D3D12_RAYTRACING_GEOMETRY_FLAG_NONE;
            geometryDesc.Triangles.VertexFormat = vertexFormatInfo.format;
            geometryDesc.Triangles.IndexFormat = indexFormat;
            geometryDesc.Triangles.Transform3x4 = NULL;

            for (const BLASCreateInfo::GeometryInfo& geometryInfo : createInfo.geometryInfos)
            {
                geometryDesc.Triangles.VertexBuffer = {
                    .StartAddress = vertexBufferGPUAddress + createInfo.vertexStride * geometryInfo.vertexOffset,
                    .StrideInBytes = createInfo.vertexStride,
                };
                geometryDesc.Triangles.VertexCount = static_cast<UINT>(geometryInfo.vertexCount);
                geometryDesc.Triangles.IndexBuffer = indexBufferGPUAddress + static_cast<Uint64>(indexStride) * geometryInfo.indexOffset;
                geometryDesc.Triangles.IndexCount = static_cast<UINT>(geometryInfo.indexCount);

                mGeometryDescs.push_back(geometryDesc);
            }

            mInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
            mInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
            mInputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
            if (createInfo.buildPreferType == ASBuildPreferType::Performance)
            {
                mInputs.Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
            }
            else if (createInfo.buildPreferType == ASBuildPreferType::BuildTime)
            {
                mInputs.Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD;
            }
            mInputs.NumDescs = static_cast<UINT>(mGeometryDescs.size());
            mInputs.pGeometryDescs = mGeometryDescs.data();

            mDevice.GetDevice()->GetRaytracingAccelerationStructurePrebuildInfo(&mInputs, &mPrebuildInfo);
            CHECK(mPrebuildInfo.ResultDataMaxSizeInBytes > 0);
            mScratchBufferSize = mPrebuildInfo.ScratchDataSizeInBytes;
            mUpdateScratchBufferSize = mPrebuildInfo.UpdateScratchDataSizeInBytes;

            D3D12_RESOURCE_DESC1 desc = {
                .Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
                .Alignment = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT,
                .Width = mPrebuildInfo.ResultDataMaxSizeInBytes,
                .Height = 1,
                .DepthOrArraySize = 1,
                .MipLevels = 1,
                .Format = DXGI_FORMAT_UNKNOWN,
                .SampleDesc = {
                    .Count = 1,
                    .Quality = 0
                },
                .Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
                .Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS | D3D12_RESOURCE_FLAG_RAYTRACING_ACCELERATION_STRUCTURE,
                .SamplerFeedbackMipRegion = { 0, 0, 0 },
            };
            mAllocation = device.GetMemoryAllocator().Allocate(D3D12_HEAP_TYPE_DEFAULT, desc, D3D12_BARRIER_LAYOUT_UNDEFINED);
            SET_DEBUG_NAME(mAllocation.resource, createInfo.debugName);
        }

        DX12BLAS::~DX12BLAS()
        {
            mDevice.GetMemoryAllocator().Free(mAllocation);
        }

        DX12TLAS::DX12TLAS(const TLASCreateInfo& createInfo, DX12Device& device)
            : TLAS(createInfo)
            , mDevice(device)
        {
        }

        DX12TLAS::~DX12TLAS()
        {
        }
    } // namespace gapi
} // namespace cube
