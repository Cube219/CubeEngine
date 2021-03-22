#pragma once

namespace cube
{
    namespace rapi
    {
        // Interface/BlendState.h
        struct BlendState;

        // Interface/Buffer.h
        struct BufferCreateInfo;
        class Buffer;
        struct VertexBufferCreateInfo;
        class VertexBuffer;
        struct IndexBufferCreateInfo;
        class IndexBuffer;
        struct ConstantBufferCreateInfo;
        class ConstantBuffer;
        struct StructuredBufferCreateInfo;
        class StructuredBuffer;

        // Interface/CommandList.h
        struct CommandListAllocateInfo;
        class CommandList;

        // Interface/DepthStencilState.h
        struct DepthStencilState;

        // Interface/Fence.h
        class Fence;

        // Interface/Framebuffer.h
        struct FramebufferCreateInfo;
        class Framebuffer;

        // Interface/InputLayout.h
        struct InputLayout;

        // Interface/PipelineState.h
        struct GraphicsPipelineStateCreateInfo;
        class GraphicsPipelineState;
        struct ComputePipelineStateCreateInfo;
        class ComputePipelineState;

        // Interface/RasterizerState.h
        struct RasterizerState;

        // Interface/RenderPass.h
        struct RenderPassAttachmentInfo;
        struct Subpass;
        struct SubpassDependency;
        struct RenderPassCreateInfo;
        class RenderPass;

        // Interface/Resource.h
        struct ResourceCreateInfo;
        class Resource;

        // Interface/Sampler.h
        struct SamplerCreateInfo;
        class Sampler;

        // Interface/Shader.h
        struct ShaderCreateInfo;
        class Shader;

        // Interface/ShaderVariables.h
        struct ShaderVariableInfo;
        class ShaderVariables;
        struct ShaderVariablesLayoutCreateInfo;
        class ShaderVariablesLayout;

        // Interface/SwapChain.h
        struct SwapChainCreateInfo;
        class SwapChain;

        // Interface/Texture.h
        struct TextureCreateInfo;
        class Texture;

        // Interface/Texture.h
        struct TextureViewCreateInfo;
        class TextureView;

        // RenderAPI.h
        struct RenderAPICreateInfo;
        class RenderAPI;
    } // namespace rapi
} // namespace cube
