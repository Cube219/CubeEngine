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

            MetalBuffer* mtlVertexBuffer = dynamic_cast<MetalBuffer*>(createInfo.vertexBuffer.get());
            CHECK(mtlVertexBuffer);
            MetalElementFormatInfo vertexFormatInfo = GetMetalElementFormatInfo(createInfo.vertexFormat);

            MetalBuffer* mtlIndexBuffer = dynamic_cast<MetalBuffer*>(createInfo.indexBuffer.get());
            MTLIndexType indexType = mtlIndexBuffer ? mtlIndexBuffer->GetMTLIndexType() : MTLIndexTypeUInt32;
            Uint64 indexStride = mtlIndexBuffer->GetStride();

            NSMutableArray<MTLAccelerationStructureGeometryDescriptor*>* mtlGeometryDescs = [NSMutableArray array];
            for (const BLASCreateInfo::GeometryInfo& geometryInfo : createInfo.geometryInfos)
            {
                MTLAccelerationStructureTriangleGeometryDescriptor* mtlGeometryDesc = [MTLAccelerationStructureTriangleGeometryDescriptor descriptor];
                mtlGeometryDesc.vertexBuffer = mtlVertexBuffer->GetMTLBuffer();
                mtlGeometryDesc.vertexBufferOffset = geometryInfo.vertexOffset * createInfo.vertexStride;
                mtlGeometryDesc.vertexStride = createInfo.vertexStride;
                mtlGeometryDesc.vertexFormat = vertexFormatInfo.attributeFormat;
                mtlGeometryDesc.indexBuffer = mtlIndexBuffer ? mtlIndexBuffer->GetMTLBuffer() : nil;
                mtlGeometryDesc.indexBufferOffset = geometryInfo.indexOffset * indexStride;
                mtlGeometryDesc.indexType = indexType;
                mtlGeometryDesc.triangleCount = mtlIndexBuffer ? geometryInfo.indexCount / 3 : geometryInfo.vertexCount / 3;
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
        }}

        MetalTLAS::~MetalTLAS()
        {
        }
    } // namespace gapi
} // namespace cube
