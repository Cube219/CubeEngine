#pragma once

#include "DX12Header.h"

#include "GAPI_Pipeline.h"

#include "DX12APIObject.h"
#include "DX12MemoryAllocator.h"

namespace cube
{
    class DX12Device;

    namespace gapi
    {
        class DX12GraphicsPipeline : public GraphicsPipeline, public DX12APIObject
        {
        public:
            DX12GraphicsPipeline(DX12Device& device, const GraphicsPipelineCreateInfo& info);
            virtual ~DX12GraphicsPipeline();

            ID3D12PipelineState* GetPipelineState() const { return mPipelineState.Get(); }

        private:
            ComPtr<ID3D12PipelineState> mPipelineState;

            Vector<SharedPtr<DX12APIObject>> mBoundObjects;
        };

        class DX12ComputePipeline : public ComputePipeline, public DX12APIObject
        {
        public:
            DX12ComputePipeline(DX12Device& device, const ComputePipelineCreateInfo& info);
            virtual ~DX12ComputePipeline();

            ID3D12PipelineState* GetPipelineState() const { return mPipelineState.Get(); }
            Uint32 GetThreadGroupSizeX() const { return mThreadGroupSizeX; }
            Uint32 GetThreadGroupSizeY() const { return mThreadGroupSizeY; }
            Uint32 GetThreadGroupSizeZ() const { return mThreadGroupSizeZ; }

        private:
            ComPtr<ID3D12PipelineState> mPipelineState;
            Uint32 mThreadGroupSizeX;
            Uint32 mThreadGroupSizeY;
            Uint32 mThreadGroupSizeZ;

            Vector<SharedPtr<DX12APIObject>> mBoundObjects;
        };

        class DX12RayTracingPipeline : public RayTracingPipeline, public DX12APIObject
        {
        public:
            DX12RayTracingPipeline(DX12Device& device, const RayTracingPipelineCreateInfo& createInfo);
            virtual ~DX12RayTracingPipeline();

            ID3D12StateObject* GetPipelineStateObject() const { return mStateObject.Get(); }
            D3D12_GPU_VIRTUAL_ADDRESS GetShaderTableGPUAddress() const { return mShaderTableAllocation.resource->GetGPUVirtualAddress(); }
            int GetRayGenShaderTableOffset() const { return mRayGenShaderTableOffset; }
            bool HasHitGroupShader() const { return mHitGroupShaderTableOffset != -1; }
            int GetHitGroupShaderTableOffset() const { return mHitGroupShaderTableOffset; }
            bool HasMissShader() const { return mMissShaderTableOffset != -1; }
            int GetMissShaderTableOffset() const { return mMissShaderTableOffset; }

        private:
            DX12Device& mDevice;

            ComPtr<ID3D12StateObject> mStateObject;
            ComPtr<ID3D12StateObjectProperties> mStateObjectProperties;

            int mRayGenShaderTableOffset = -1;
            int mHitGroupShaderTableOffset = -1;
            int mMissShaderTableOffset = -1;
            DX12Allocation mShaderTableAllocation;

            Vector<SharedPtr<DX12APIObject>> mBoundObjects;
        };
    } // namespace gapi
} // namespace cube
