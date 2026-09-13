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

            MTLPrimitiveAccelerationStructureDescriptor* GetDesc() const { return mDesc; }
            id<MTLAccelerationStructure> GetMetalAS() const { return mAS; }

        private:
            MetalDevice& mDevice;

            MTLPrimitiveAccelerationStructureDescriptor* mDesc;
            id<MTLAccelerationStructure> mAS;
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
