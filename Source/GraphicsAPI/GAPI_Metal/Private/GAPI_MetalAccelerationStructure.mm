#include "GAPI_MetalAccelerationStructure.h"

#include "Allocator/FrameAllocator.h"
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

            MetalBuffer* metalIndexBuffer = dynamic_cast<MetalBuffer*>(createInfo.indexBuffer.get());
            MTLIndexType indexType = metalIndexBuffer ? metalIndexBuffer->GetMTLIndexType() : MTLIndexTypeUInt32;
            Uint64 indexStride = metalIndexBuffer ? metalIndexBuffer->GetStride() : 0;

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
            if (createInfo.buildPreferType == AccelerationStructureBuildPreferType::Performance)
            {
                mDesc.usage |= MTLAccelerationStructureUsagePreferFastIntersection;
            }
            else if (createInfo.buildPreferType == AccelerationStructureBuildPreferType::BuildTime)
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

            mBLAS = [mDevice.GetMTLDevice() newAccelerationStructureWithSize:sizes.accelerationStructureSize];
            mBLAS.label = String_Convert<NSString*>(createInfo.debugName);
        }}

        MetalBLAS::~MetalBLAS()
        {
            mBLAS = nil;
            mDesc = nil;
        }

        MetalTLAS::MetalTLAS(const TLASCreateInfo& createInfo, MetalDevice& device)
            : TLAS(createInfo)
            , mDevice(device)
        { @autoreleasepool {
            CHECK(device.IsHWRTSupported());

            mInstanceDescriptor = [MTLInstanceAccelerationStructureDescriptor descriptor];
            mInstanceDescriptor.instanceCount = createInfo.instances.size();
            if (createInfo.buildPreferType == AccelerationStructureBuildPreferType::Performance)
            {
                mInstanceDescriptor.usage |= MTLAccelerationStructureUsagePreferFastIntersection;
            }
            else if (createInfo.buildPreferType == AccelerationStructureBuildPreferType::BuildTime)
            {
                mInstanceDescriptor.usage |= MTLAccelerationStructureUsagePreferFastBuild;
            }
            if (createInfo.allowUpdate)
            {
                mInstanceDescriptor.usage |= MTLAccelerationStructureUsageRefit;
            }

            MTLAccelerationStructureSizes sizes = [mDevice.GetMTLDevice() accelerationStructureSizesWithDescriptor:mInstanceDescriptor];
            mScratchBufferSize = sizes.buildScratchBufferSize;
            mUpdateScratchBufferSize = sizes.refitScratchBufferSize;

            mTLAS = [mDevice.GetMTLDevice() newAccelerationStructureWithSize:sizes.accelerationStructureSize];
            mTLAS.label = String_Convert<NSString*>(createInfo.debugName);

            mInstances.resize(createInfo.instances.size());
            UpdateInstances(createInfo.instances);
            mInstanceDescBufferSize = sizeof(MTLAccelerationStructureInstanceDescriptor) * createInfo.instances.size();
        }}

        MetalTLAS::~MetalTLAS()
        {
            mTLAS = nil;
            mInstances.clear();
        }

        void MetalTLAS::UpdateInstances(ConstArrayView<BLASInstance> instances)
        {
            CHECK(mInstances.size() == instances.size());
            mInstances = { instances.begin(), instances.end() };

            NSMutableArray<id<MTLAccelerationStructure>>* mtlAccelerationStructures = [NSMutableArray array];
            for (const BLASInstance& instance : mInstances)
            {
                MetalBLAS* metalBLAS = dynamic_cast<MetalBLAS*>(instance.blas.get());
                CHECK(metalBLAS);

                [mtlAccelerationStructures addObject:metalBLAS->GetMTLAccelerationStructure()];
            }
            mInstanceDescriptor.instancedAccelerationStructures = mtlAccelerationStructures;
        }

        void MetalTLAS::WriteInstanceDescriptor(MetalBuffer* instanceDescBuffer)
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
                mtlInstanceDesc.mask = 0xFF;
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
