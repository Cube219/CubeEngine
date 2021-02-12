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

            const InputLayout* inputLayouts;
            Uint32 numInputLayouts;

            RasterizerState rasterizerState;
            PrimitiveTopology topology = PrimitiveTopology::TriangleList;
            DepthStencilState depthStencilState;

            Uint32 numRenderTargets;
            BlendState blendStates[MAX_RENDER_TARGET_NUM];
            TextureFormat renderTargetFormats[MAX_RENDER_TARGET_NUM];
            TextureFormat depthStencilFormat;

            // TODO: RenderPass

            ShaderVariablesLayout** shaderVariablesLayouts;
            Uint32 numShaderVariablesLayouts;

            const char* debugName = "";
        };

        class GraphicsPipelineState
        {
        public:
            GraphicsPipelineState() = default;
            virtual ~GraphicsPipelineState() = default;
        };
    } // namespace rapi
} // namespace cube
