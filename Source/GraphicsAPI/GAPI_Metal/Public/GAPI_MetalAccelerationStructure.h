#pragma once

#include "MetalHeader.h"

#include "GAPI_AccelerationStructure.h"

namespace cube
{
    class MetalDevice;
    
    namespace gapi
    {
        class MetalBuffer;

        class MetalBLAS : public BLAS
        {
        public:
            MetalBLAS(const BLASCreateInfo& createInfo, MetalDevice& device);
            virtual ~MetalBLAS();

            MTLPrimitiveAccelerationStructureDescriptor* GetDesc() const { return mDesc; }
            id<MTLAccelerationStructure> GetMTLAccelerationStructure() const { return mBLAS; }

        private:
            MetalDevice& mDevice;

            MTLPrimitiveAccelerationStructureDescriptor* mDesc;
            id<MTLAccelerationStructure> mBLAS;
        };

        class MetalTLAS : public TLAS
        {
        public:
            MetalTLAS(const TLASCreateInfo& createInfo, MetalDevice& device);
            virtual ~MetalTLAS();

            virtual void UpdateInstances(ConstArrayView<BLASInstance> instances) override;

            MTLInstanceAccelerationStructureDescriptor* GetInstanceDescriptor() const { return mInstanceDescriptor; }
            id<MTLAccelerationStructure> GetMTLAccelerationStructure() const { return mTLAS; }

            void WriteInstanceDescriptor(MetalBuffer* instanceDescBuffer);

            id<MTLResidencySet> GetTLASAndBLASInstancesResidencySet() const { return mTLASAndBLASInstancesResidencySet; }

        private:
            void UpdateResidencySet();

            MetalDevice& mDevice;

            Vector<BLASInstance> mInstances;
            MTLInstanceAccelerationStructureDescriptor* mInstanceDescriptor;

            id<MTLAccelerationStructure> mTLAS;

            id<MTLResidencySet> mTLASAndBLASInstancesResidencySet;
        };
    } // namespace gapi
} // namespace cube
