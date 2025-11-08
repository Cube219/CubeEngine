#pragma once

#include "MetalHeader.h"

#include "GAPI_Pipeline.h"

namespace cube
{
    class MetalDevice;

    namespace gapi
    {
        class MetalGraphicsPipeline : public GraphicsPipeline
        {
        public:
            MetalGraphicsPipeline(const GraphicsPipelineCreateInfo& info, MetalDevice& device);
            virtual ~MetalGraphicsPipeline();

            id<MTLRenderPipelineState> GetMTLRenderPipelineState() const { return mPipelineState; }
            id<MTLDepthStencilState> GetMTLDepthStencilState() const { return mDepthStencilState; }

        private:
            id<MTLRenderPipelineState> mPipelineState;
            id<MTLDepthStencilState> mDepthStencilState;

            MTLTriangleFillMode mFillMode;
            MTLCullMode mCullMode;
            MTLWinding mWinding;
        };

        class MetalComputePipeline : public ComputePipeline
        {
        public:
            MetalComputePipeline(const ComputePipelineCreateInfo& info, MetalDevice& device);
            virtual ~MetalComputePipeline();

            id<MTLComputePipelineState> GetMTLComputePipelineState() const { return mPipelineState; }

        private:
            id<MTLComputePipelineState> mPipelineState;
        };
    } // namespace gapi
} // namespace cube
