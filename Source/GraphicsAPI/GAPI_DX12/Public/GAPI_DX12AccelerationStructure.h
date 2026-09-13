#pragma once

#include "DX12Header.h"

#include "DX12APIObject.h"
#include "DX12MemoryAllocator.h"
#include "GAPI_AccelerationStructure.h"

namespace cube
{
    class DX12Device;

    namespace gapi
    {
        class DX12BLAS : public BLAS, public DX12APIObject
        {
        public:
            DX12BLAS(const BLASCreateInfo& createInfo, DX12Device& device);
            virtual ~DX12BLAS();

        private:
            DX12Device& mDevice;

            D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO mPrebuildInfo;
            DX12Allocation mAllocation;
        };

        class DX12TLAS : public TLAS, public DX12APIObject
        {
        };
    } // namespace gapi
} // namespace cube
