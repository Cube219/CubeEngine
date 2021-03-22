#pragma once

#include "VulkanAPIHeader.h"

#include "RenderAPIs/RenderAPI/RenderAPI.h"
#include "VulkanDevice.h"

namespace cube
{
    namespace rapi
    {
        extern "C" VK_API_EXPORT RenderAPI* CreateRenderAPI();

        class VK_API_EXPORT VulkanAPI : public RenderAPI
        {
        public:
            VulkanAPI() {}
            virtual ~VulkanAPI() {}

            virtual void Initialize(const RenderAPICreateInfo& info) override;
            virtual void Shutdown() override;

            virtual SPtr<VertexBuffer> CreateVertexBuffer(const VertexBufferCreateInfo& info) override;
            virtual SPtr<IndexBuffer> CreateIndexBuffer(const IndexBufferCreateInfo& info) override;
            virtual SPtr<CommandList> AllocateCommandList(const CommandListAllocateInfo& info) override;
            virtual SPtr<Framebuffer> CreateFramebuffer(const FramebufferCreateInfo& info) override;
            virtual SPtr<GraphicsPipelineState> CreateGraphicsPipelineState(const GraphicsPipelineStateCreateInfo& info) override;
            virtual SPtr<ComputePipelineState> CreateComputePipelineState(const ComputePipelineStateCreateInfo& info) override;
            virtual SPtr<RenderPass> CreateRenderPass(const RenderPassCreateInfo& info) override;
            virtual SPtr<Sampler> CreateSampler(const SamplerCreateInfo& info) override;
            virtual SPtr<Shader> CreateShader(const ShaderCreateInfo& info) override;
            virtual SPtr<ShaderVariablesLayout> CreateShaderVariablesLayout(const ShaderVariablesLayoutCreateInfo& info) override;
            virtual SPtr<SwapChain> CreateSwapChain(const SwapChainCreateInfo& info) override;
            virtual SPtr<Texture> CreateTexture(const TextureCreateInfo& info) override;

            VkInstance GetInstance() const { return mInstance; }
            VulkanDevice* GetDevice() { return mDevice; }

        private:
            void CreateInstance(bool enableDebugLayer);
            void GetDevices(const RenderAPICreateInfo& info);

        
            VkInstance mInstance;
            Vector<VulkanDevice*> mAllDevices;
            VulkanDevice* mDevice;
        };
    } // namespace rapi
} // namespace cube
