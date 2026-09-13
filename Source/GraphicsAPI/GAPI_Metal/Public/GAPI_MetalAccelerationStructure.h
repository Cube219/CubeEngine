#pragma once

#include "MetalHeader.h"

#include "GAPI_AccelerationStructure.h"

namespace cube
{
    class MetalDevice;
    
    namespace gapi
    {
        class MetalBLAS : public BLAS
        {
        public:
            MetalBLAS(const BLASCreateInfo& createInfo, MetalDevice& device);
            virtual ~MetalBLAS();

        private:
            MetalDevice& mDevice;

            MTLPrimitiveAccelerationStructureDescriptor* mDesc;
            id<MTLAccelerationStructure> mAS;
            id<MTLBuffer> mScratchBuffer;
        };

        class MetalTLAS : public TLAS
        {
        public:
            MetalTLAS(const TLASCreateInfo& createInfo, MetalDevice& device);
            virtual ~MetalTLAS();

        private:
            MetalDevice& mDevice;
        };
    } // namespace gapi
} // namespace cube
