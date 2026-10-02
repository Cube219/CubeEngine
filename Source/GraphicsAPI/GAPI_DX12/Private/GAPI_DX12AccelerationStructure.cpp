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
            mVertexBufferToBuild = createInfo.vertexBuffer;

            DX12Buffer* dx12IndexBuffer = dynamic_cast<DX12Buffer*>(createInfo.indexBuffer.get());
            D3D12_GPU_VIRTUAL_ADDRESS indexBufferGPUAddress = dx12IndexBuffer ? dx12IndexBuffer->GetGPUAddress() : NULL;
            DXGI_FORMAT indexFormat = dx12IndexBuffer ? dx12IndexBuffer->GetIndexFormat() : DXGI_FORMAT_UNKNOWN;
            const Uint32 indexStride = dx12IndexBuffer ? dx12IndexBuffer->GetStride() : 0;
            mIndexBufferToBuild = createInfo.indexBuffer;

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
            if (createInfo.allowUpdate)
            {
                mInputs.Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;
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
            CHECK(mDevice.IsHWRTSupported());

            mInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
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
            if (createInfo.allowUpdate)
            {
                mInputs.Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;
            }
            mInputs.NumDescs = static_cast<UINT>(createInfo.instances.size());

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

            mInstances = { createInfo.instances.begin(), createInfo.instances.end() };
            mInstanceDescBufferSize = sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * mInstances.size();
        }

        DX12TLAS::~DX12TLAS()
        {
            mDevice.GetMemoryAllocator().Free(mAllocation);
        }

        void DX12TLAS::UpdateInstances(ConstArrayView<BLASInstance> instances)
        {
            CHECK(mInstances.size() == instances.size());
            mInstances = { instances.begin(), instances.end() };
        }

        void DX12TLAS::WriteInstanceDescs(DX12Buffer& instanceBuffer)
        {
            FrameVector<D3D12_RAYTRACING_INSTANCE_DESC> dx12InstanceDescs;
            dx12InstanceDescs.reserve(mInstances.size());
            for (BLASInstance& blasInstance : mInstances)
            {
                DX12BLAS* dx12BLAS = dynamic_cast<DX12BLAS*>(blasInstance.blas.get());
                CHECK(dx12BLAS);

                Matrix transformT = blasInstance.transform.Transposed();
                Float4 row0 = transformT.GetRow(0).GetFloat4();
                Float4 row1 = transformT.GetRow(1).GetFloat4();
                Float4 row2 = transformT.GetRow(2).GetFloat4();

                D3D12_RAYTRACING_INSTANCE_DESC& dx12InstanceDesc = dx12InstanceDescs.emplace_back();
                dx12InstanceDesc.Transform[0][0] = row0.x;
                dx12InstanceDesc.Transform[0][1] = row0.y;
                dx12InstanceDesc.Transform[0][2] = row0.z;
                dx12InstanceDesc.Transform[0][3] = row0.w;
                dx12InstanceDesc.Transform[1][0] = row1.x;
                dx12InstanceDesc.Transform[1][1] = row1.y;
                dx12InstanceDesc.Transform[1][2] = row1.z;
                dx12InstanceDesc.Transform[1][3] = row1.w;
                dx12InstanceDesc.Transform[2][0] = row2.x;
                dx12InstanceDesc.Transform[2][1] = row2.y;
                dx12InstanceDesc.Transform[2][2] = row2.z;
                dx12InstanceDesc.Transform[2][3] = row2.w;
                dx12InstanceDesc.InstanceID = 0;
                dx12InstanceDesc.InstanceMask = 1;
                dx12InstanceDesc.InstanceContributionToHitGroupIndex = blasInstance.hitGroupIndex;
                dx12InstanceDesc.Flags = D3D12_RAYTRACING_INSTANCE_FLAG_TRIANGLE_FRONT_COUNTERCLOCKWISE;
                dx12InstanceDesc.AccelerationStructure = dx12BLAS->GetGPUAddress();
            }

            CHECK(instanceBuffer.GetUsage() == ResourceUsage::CPUtoGPU);
            CHECK(instanceBuffer.GetSize() >= sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * dx12InstanceDescs.size());

            D3D12_RAYTRACING_INSTANCE_DESC* pInstanceBufferCPU = reinterpret_cast<D3D12_RAYTRACING_INSTANCE_DESC*>(instanceBuffer.Map());
            memcpy(pInstanceBufferCPU, dx12InstanceDescs.data(), sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * dx12InstanceDescs.size());
            instanceBuffer.Unmap();
        }
    } // namespace gapi
} // namespace cube
