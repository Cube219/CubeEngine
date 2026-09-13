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
            const Uint32 indexStride = dx12Buffer->GetStride();
            CHECK_FORMAT(indexStride == 4 || indexStride == 2, "Index buffer's stride must be 2(16bits) or 4(32bits).");

            FrameVector<D3D12_RAYTRACING_GEOMETRY_DESC> geometryDescs(createInfo.geometryInfos.size());

            D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc;
            geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
            geometryDesc.Flags = createInfo.isOpaque ? D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE : D3D12_RAYTRACING_GEOMETRY_FLAG_NONE;
            geometryDesc.Triangles.VertexFormat = vertexFormatInfo.format;
            geometryDesc.Triangles.IndexFormat = indexFormatInfo.format;
            geometryDesc.Triangles.Transform3x4 = NULL;

            for (int i = 0; i < static_cast<int>(createInfo.geometryInfos.size()); ++i)
            {
                const BLASCreateInfo::GeometryInfo& geometryInfo = createInfo.geometryInfos[i];

                geometryDesc.Triangles.VertexBuffer = {
                    .StartAddress = vertexBufferGPUAddress + createInfo.vertexStride * geometryInfo.vertexOffset,
                    .StrideInBytes = createInfo.vertexStride,
                };
                geometryDesc.Triangles.VertexCount = static_cast<UINT>(geometryInfo.vertexCount);
                geometryDesc.Triangles.IndexBuffer = indexBufferGPUAddress + static_cast<Uint64>(indexFormatInfo.bytes) * geometryInfo.indexOffset;
                geometryDesc.Triangles.IndexCount = static_cast<UINT>(geometryInfo.indexCount);

                geometryDescs[i] = geometryDesc;
            }

            D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc = {};
            D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS& bottomLevelInputs = buildDesc.Inputs;
            bottomLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
            bottomLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
            bottomLevelInputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
            if (createInfo.allowUpdate)
            {
                bottomLevelInputs.Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;
            }
            bottomLevelInputs.NumDescs = static_cast<UINT>(geometryDescs.size());
            bottomLevelInputs.pGeometryDescs = geometryDescs.data();

            mDevice.GetDevice()->GetRaytracingAccelerationStructurePrebuildInfo(&bottomLevelInputs, &mPrebuildInfo);
            CHECK(mPrebuildInfo.ResultDataMaxSizeInBytes > 0);

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
                .Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
                .SamplerFeedbackMipRegion = { 0, 0, 0 },
            };
            mAllocation = device.GetMemoryAllocator().Allocate(D3D12_HEAP_TYPE_DEFAULT, desc, D3D12_BARRIER_LAYOUT_UNDEFINED);
            SET_DEBUG_NAME(mAllocation.resource, createInfo.debugName);
        }

        DX12BLAS::~DX12BLAS()
        {
            mDevice.GetMemoryAllocator().Free(mAllocation);
        }
    } // namespace gapi
} // namespace cube
