#pragma once

#include "../VulkanAPIHeader.h"

#include "RenderAPIs/RenderAPI/Interface/PipelineState.h"

namespace cube
{
    namespace rapi
    {
        VkVertexInputRate ConvertToVkVertexInputRate(InputRate inputRate);
        VkPrimitiveTopology ConvertToVkPrimitiveTopology(PrimitiveTopology topology);
        VkPolygonMode ConvertToVkPolygonMode(FillMode fillMode);
        VkCullModeFlags ConvertToVkCullModeFlags(CullMode cullMode);
        VkFrontFace ConvertToVkFrontFace(FrontFace frontFace);
        VkCompareOp ConvertToVkCompareOp(CompareFunction compareFunction);
        VkStencilOp ConvertToVkStencilOp(StencilOperator stencilOperator);
        VkStencilOpState ConvertToVkStencilOpState(CompareFunction compareFunction, StencilOperator stencilFailOp, StencilOperator stencilPassOp,
            StencilOperator stencilDepthFailOp, Uint8 stencilReadMask, Uint8 stencilWriteMask);
        VkBlendFactor ConvertToVkBlendFactor(BlendFactor blendFactor);
        VkBlendOp ConvertToVkBlendOp(BlendOperator blendOperator);
        VkColorComponentFlags ConvertToVkColorComponentFlags(ColorWriteMaskFlags colorWriteMaskFlags);

        class GraphicsPipelineStateVk : public GraphicsPipelineState
        {
        public:
            GraphicsPipelineStateVk(VulkanDevice& device, const GraphicsPipelineStateCreateInfo& info);
            virtual ~GraphicsPipelineStateVk();

            VkPipeline GetHandle() const { return mPipeline; }
            VkPipelineLayout GetPipelineLayout() const { return mPipelineLayout; }
            
        private:
            VulkanDevice& mDevice;

            VkPipeline mPipeline;
            VkPipelineLayout mPipelineLayout;
        };

        class ComputePipelineStateVk : public ComputePipelineState
        {
        public:
            ComputePipelineStateVk(VulkanDevice& device, const ComputePipelineStateCreateInfo& info);
            virtual ~ComputePipelineStateVk();

            VkPipeline GetHandle() const { return mPipeline; }
            VkPipelineLayout GetPipelineLayout() const { return mPipelineLayout; }

        private:
            VulkanDevice& mDevice;

            VkPipeline mPipeline;
            VkPipelineLayout mPipelineLayout;
        };
    } // namespace rapi
} // namespace cube
