#pragma once

#include "../RenderAPIHeader.h"

#include "BlendState.h"
#include "DepthStencilState.h"
#include "InputLayout.h"
#include "RasterizerState.h"

namespace cube
{
    namespace rapi
    {
        constexpr int MAX_RENDER_TARGET_NUM = 8;
        
        struct GraphicsPipelineStateCreateInfo
        {
            Shader* vertexShader = nullptr;
            Shader* pixelShader = nullptr;
            Shader* domainShader = nullptr;
            Shader* hullShader = nullptr;
            Shader* geometryShader = nullptr;

            const InputLayout* inputLayouts = nullptr;
            Uint32 numInputLayouts = 0;

            RasterizerState rasterizerState;
            PrimitiveTopology topology = PrimitiveTopology::TriangleList;
            DepthStencilState depthStencilState;

            Uint32 numRenderTargets = 0;
            BlendState blendStates[MAX_RENDER_TARGET_NUM];
            TextureFormat renderTargetFormats[MAX_RENDER_TARGET_NUM];
            TextureFormat depthStencilFormat;

            RenderPass* renderPass;

            Uint32 numShaderVariablesLayouts = 0;
            ShaderVariablesLayout** shaderVariablesLayouts = nullptr;

            const char* debugName = "";
        };

        class GraphicsPipelineState
        {
        public:
            GraphicsPipelineState() = default;
            virtual ~GraphicsPipelineState() = default;
        };

        struct ComputePipelineStateCreateInfo
        {
            Shader* shader;

            Uint32 numShaderVariablesLayouts;
            ShaderVariablesLayout** shaderVariablesLayouts;

            const char* debugName = "";
        };

        class ComputePipelineState
        {
        public:
            ComputePipelineState() = default;
            virtual ~ComputePipelineState() = default;
        };
    } // namespace rapi
} // namespace cube
