#include "GAPI_MetalAccelerationStructure.h"

#import "Allocator/FrameAllocator.h"
#include "Checker.h"
#include "GAPI_MetalBuffer.h"
#include "MacOS/MacOSString.h"
#include "MetalDevice.h"
#include "MetalTypes.h"

namespace cube
{
    namespace gapi
    {
        MetalBLAS::MetalBLAS(const BLASCreateInfo& createInfo, MetalDevice& device)
            : BLAS(createInfo)
            , mDevice(device)
        { @autoreleasepool {
            CHECK(device.IsHWRTSupported());

            MetalBuffer* metalVertexBuffer = dynamic_cast<MetalBuffer*>(createInfo.vertexBuffer.get());
            CHECK(metalVertexBuffer);
            MetalElementFormatInfo vertexFormatInfo = GetMetalElementFormatInfo(createInfo.vertexFormat);
            mVertexBufferToBuild = createInfo.vertexBuffer;

            MetalBuffer* metalIndexBuffer = dynamic_cast<MetalBuffer*>(createInfo.indexBuffer.get());
            MTLIndexType indexType = metalIndexBuffer ? metalIndexBuffer->GetMTLIndexType() : MTLIndexTypeUInt32;
            Uint64 indexStride = metalIndexBuffer ? metalIndexBuffer->GetStride() : 0;
            mIndexBufferToBuild = createInfo.indexBuffer;

            NSMutableArray<MTLAccelerationStructureGeometryDescriptor*>* mtlGeometryDescs = [NSMutableArray array];
            for (const BLASCreateInfo::GeometryInfo& geometryInfo : createInfo.geometryInfos)
            {
                MTLAccelerationStructureTriangleGeometryDescriptor* mtlGeometryDesc = [MTLAccelerationStructureTriangleGeometryDescriptor descriptor];
                mtlGeometryDesc.vertexBuffer = metalVertexBuffer->GetMTLBuffer();
                mtlGeometryDesc.vertexBufferOffset = geometryInfo.vertexOffset * createInfo.vertexStride;
                mtlGeometryDesc.vertexStride = createInfo.vertexStride;
                mtlGeometryDesc.vertexFormat = vertexFormatInfo.attributeFormat;
                mtlGeometryDesc.indexBuffer = metalIndexBuffer ? metalIndexBuffer->GetMTLBuffer() : nil;
                mtlGeometryDesc.indexBufferOffset = geometryInfo.indexOffset * indexStride;
                mtlGeometryDesc.indexType = indexType;
                mtlGeometryDesc.triangleCount = metalIndexBuffer ? geometryInfo.indexCount / 3 : geometryInfo.vertexCount / 3;
                mtlGeometryDesc.opaque = createInfo.isOpaque;

                [mtlGeometryDescs addObject:mtlGeometryDesc];
            }

            mDesc = [MTLPrimitiveAccelerationStructureDescriptor descriptor];
            mDesc.geometryDescriptors = mtlGeometryDescs;
            mDesc.usage = MTLAccelerationStructureUsageNone;
            if (createInfo.buildPreferType == ASBuildPreferType::Performance)
            {
                mDesc.usage |= MTLAccelerationStructureUsagePreferFastIntersection;
            }
            else if (createInfo.buildPreferType == ASBuildPreferType::BuildTime)
            {
                mDesc.usage |= MTLAccelerationStructureUsagePreferFastBuild;
            }
            if (createInfo.allowUpdate)
            {
                mDesc.usage |= MTLAccelerationStructureUsageRefit;
            }

            MTLAccelerationStructureSizes sizes = [mDevice.GetMTLDevice() accelerationStructureSizesWithDescriptor:mDesc];
            mScratchBufferSize = sizes.buildScratchBufferSize;
            mUpdateScratchBufferSize = sizes.refitScratchBufferSize;

            mAS = [mDevice.GetMTLDevice() newAccelerationStructureWithSize:sizes.accelerationStructureSize];
            mAS.label = String_Convert<NSString*>(createInfo.debugName);
        }}

        MetalBLAS::~MetalBLAS()
        {
            mAS = nil;
            // cube_TODO: Check actually released.
            mDesc = nil;
        }

        MetalTLAS::MetalTLAS(const TLASCreateInfo& createInfo, MetalDevice& device)
            : TLAS(createInfo)
            , mDevice(device)
        { @autoreleasepool {
            CHECK(device.IsHWRTSupported());

            mPartialDesc = [MTLInstanceAccelerationStructureDescriptor descriptor];
            mPartialDesc.instanceCount = createInfo.instances.size();
            if (createInfo.buildPreferType == ASBuildPreferType::Performance)
            {
                mPartialDesc.usage |= MTLAccelerationStructureUsagePreferFastIntersection;
            }
            else if (createInfo.buildPreferType == ASBuildPreferType::BuildTime)
            {
                mPartialDesc.usage |= MTLAccelerationStructureUsagePreferFastBuild;
            }
            if (createInfo.allowUpdate)
            {
                mPartialDesc.usage |= MTLAccelerationStructureUsageRefit;
            }

            MTLAccelerationStructureSizes sizes = [mDevice.GetMTLDevice() accelerationStructureSizesWithDescriptor:mPartialDesc];
            mScratchBufferSize = sizes.buildScratchBufferSize;
            mUpdateScratchBufferSize = sizes.refitScratchBufferSize;

            mAS = [mDevice.GetMTLDevice() newAccelerationStructureWithSize:sizes.accelerationStructureSize];
            mAS.label = String_Convert<NSString*>(createInfo.debugName);

            mInstances = { createInfo.instances.begin(), createInfo.instances.end() };
            mInstanceDescBufferSize = sizeof(MTLAccelerationStructureInstanceDescriptor) * createInfo.instances.size();
        }}

        MetalTLAS::~MetalTLAS()
        {
            mAS = nil;
            mInstances.clear();
        }

        void MetalTLAS::UpdateInstances(ConstArrayView<BLASInstance> instances)
        {
            CHECK(mInstances.size() == instances.size());
            mInstances = { instances.begin(), instances.end() };
        }

        MTLInstanceAccelerationStructureDescriptor* MetalTLAS::GetDesc() const
        {
            // TODO: Caching.
            MTLInstanceAccelerationStructureDescriptor* newDesc = [MTLInstanceAccelerationStructureDescriptor descriptor];
            newDesc.instanceCount = mPartialDesc.instanceCount;
            newDesc.usage = mPartialDesc.usage;

            NSMutableArray<id<MTLAccelerationStructure>>* mtlASes = [NSMutableArray array];
            for (const BLASInstance& instance : mInstances)
            {
                MetalBLAS* metalBLAS = dynamic_cast<MetalBLAS*>(instance.blas.get());
                CHECK(metalBLAS);

                [mtlASes addObject:metalBLAS->GetMetalAS()];
            }
            newDesc.instancedAccelerationStructures = mtlASes;

            return newDesc;
        }

        void MetalTLAS::WriteInstanceDescs(MetalBuffer* instanceDescBuffer)
        {
            FrameVector<MTLAccelerationStructureInstanceDescriptor> mtlInstanceDescs;
            mtlInstanceDescs.reserve(mInstances.size());
            for (BLASInstance& blasInstance : mInstances)
            {
                MetalBLAS* metalBLAS = dynamic_cast<MetalBLAS*>(blasInstance.blas.get());
                CHECK(metalBLAS);

                Float4 row0 = blasInstance.transform.GetRow(0).GetFloat4();
                Float4 row1 = blasInstance.transform.GetRow(1).GetFloat4();
                Float4 row2 = blasInstance.transform.GetRow(2).GetFloat4();
                Float4 row3 = blasInstance.transform.GetRow(3).GetFloat4();

                MTLAccelerationStructureInstanceDescriptor& mtlInstanceDesc = mtlInstanceDescs.emplace_back();
                mtlInstanceDesc.transformationMatrix[0] = { row0.x, row0.y, row0.z };
                mtlInstanceDesc.transformationMatrix[1] = { row1.x, row1.y, row1.z };
                mtlInstanceDesc.transformationMatrix[2] = { row2.x, row2.y, row2.z };
                mtlInstanceDesc.transformationMatrix[3] = { row3.x, row3.y, row3.z };
                mtlInstanceDesc.options = MTLAccelerationStructureInstanceOptionTriangleFrontFacingWindingCounterClockwise;
                mtlInstanceDesc.mask = 0xFFFFFFFF;
                mtlInstanceDesc.intersectionFunctionTableOffset = blasInstance.hitGroupIndex;
                mtlInstanceDesc.accelerationStructureIndex = mtlInstanceDescs.size() - 1;
            }

            CHECK(sizeof(MTLAccelerationStructureInstanceDescriptor) * mtlInstanceDescs.size() <= instanceDescBuffer->GetSize());
            CHECK(instanceDescBuffer->GetUsage() == ResourceUsage::CPUtoGPU);

            MTLAccelerationStructureInstanceDescriptor* pDstData = (MTLAccelerationStructureInstanceDescriptor*)instanceDescBuffer->Map();
            memcpy(pDstData, mtlInstanceDescs.data(), sizeof(MTLAccelerationStructureInstanceDescriptor) * mtlInstanceDescs.size());
            instanceDescBuffer->Unmap();
        }
    } // namespace gapi
} // namespace cube
