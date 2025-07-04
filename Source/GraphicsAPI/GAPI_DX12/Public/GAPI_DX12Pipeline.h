#pragma once

#include "DX12Header.h"

#include "GAPI_Pipeline.h"

#include "DX12APIObject.h"

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

        private:
            ComPtr<ID3D12PipelineState> mPipelineState;
        };
    } // namespace gapi
} // namespace cube
