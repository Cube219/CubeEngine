#pragma once

#include "RenderAPIHeader.h"

namespace cube
{
    namespace rapi
    {
        struct RenderAPICreateInfo
        {
            bool enableDebugLayer = false;

            Uint32 numGraphicsCommandPools = 4;
            Uint32 numComputeCommandPools = 1;
            Uint32 numTransferCommandPools = 4;
        };

        class RenderAPI
        {
        public:
            RenderAPI() {}
            virtual ~RenderAPI() {}

            virtual void Initialize(const RenderAPICreateInfo& info) = 0;
            virtual void Shutdown() = 0;

            virtual SPtr<VertexBuffer> CreateVertexBuffer(const VertexBufferCreateInfo& info) = 0;
            virtual SPtr<IndexBuffer> CreateIndexBuffer(const IndexBufferCreateInfo& info) = 0;
            virtual SPtr<CommandList> AllocateCommandList(const CommandListAllocateInfo& info) = 0;
            virtual SPtr<Framebuffer> CreateFramebuffer(const FramebufferCreateInfo& info) = 0;
            virtual SPtr<GraphicsPipelineState> CreateGraphicsPipelineState(const GraphicsPipelineStateCreateInfo& info) = 0;
            virtual SPtr<ComputePipelineState> CreateComputePipelineState(const ComputePipelineStateCreateInfo& info) = 0;
            virtual SPtr<RenderPass> CreateRenderPass(const RenderPassCreateInfo& info) = 0;
            virtual SPtr<Sampler> CreateSampler(const SamplerCreateInfo& info) = 0;
            virtual SPtr<Shader> CreateShader(const ShaderCreateInfo& info) = 0;
            virtual SPtr<ShaderVariablesLayout> CreateShaderVariablesLayout(const ShaderVariablesLayoutCreateInfo& info) = 0;
            virtual SPtr<SwapChain> CreateSwapChain(const SwapChainCreateInfo& info) = 0;
            virtual SPtr<Texture> CreateTexture(const TextureCreateInfo& info) = 0;
        };
    } // namespace rapi
} // namespace cube
