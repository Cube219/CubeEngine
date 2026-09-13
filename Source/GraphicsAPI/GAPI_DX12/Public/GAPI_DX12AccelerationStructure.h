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
        class DX12Buffer;

        class DX12BLAS : public BLAS, public DX12APIObject
        {
        public:
            DX12BLAS(const BLASCreateInfo& createInfo, DX12Device& device);
            virtual ~DX12BLAS();

            D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS GetInputs() const { return mInputs; }
            D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress() const { return mAllocation.resource->GetGPUVirtualAddress(); }

        private:
            DX12Device& mDevice;

            Vector<D3D12_RAYTRACING_GEOMETRY_DESC> mGeometryDescs;
            D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS mInputs;
            D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO mPrebuildInfo;
            DX12Allocation mAllocation;
        };

        class DX12TLAS : public TLAS, public DX12APIObject
        {
        public:
            DX12TLAS(const TLASCreateInfo& createInfo, DX12Device& device);
            virtual ~DX12TLAS();

        private:
            DX12Device& mDevice;
        };
    } // namespace gapi
} // namespace cube
