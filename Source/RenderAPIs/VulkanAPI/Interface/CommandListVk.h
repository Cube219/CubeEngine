#pragma once

#include "../VulkanAPIHeader.h"

#include "RenderAPIs/RenderAPI/Interface/CommandList.h"

#include "../VulkanCommandPoolManager.h"

namespace cube
{
    namespace rapi
    {
        class CommandListVk : public CommandList
        {
        public:
            CommandListVk(VulkanDevice& device, VulkanCommandPoolManager& poolManager, const CommandListAllocateInfo& info, VulkanCommandBuffer commandBuffer);
            virtual ~CommandListVk();

            VulkanCommandBuffer& GetCommandBuffer() { return mCommandBuffer; }
            VkCommandBuffer GetVkCommandBuffer() const { return mCommandBuffer.handle; }
            CommandListType GetType() const { return mCommandBuffer.type; }
            Uint32 GetPoolIndex() const { return mCommandBuffer.poolIndex; }

            virtual void Begin() override;
            virtual void BeginSubWithRenderPass(RenderPass* pRenderPass, Framebuffer* pFramebuffer, Uint32 subpassIndex) override;
            virtual void End() override;
            virtual void Reset() override;

            virtual void SetViewports(Uint32 numViewports, const Viewport* pViewports) override;
            virtual void SetScissors(Uint32 numScissors, const Rect2D* pScissors) override;

            virtual void SetPipelineState(GraphicsPipelineState* pipelineState) override;

            virtual void BeginRenderPass(RenderPass* pRenderPass, Framebuffer* pFramebuffer, Uint32 numClearValues, ClearValue* pClearValues, bool useSubCommandList) override;
            virtual void NextSubpass(bool useSubCommandList) override;
            virtual void EndRenderPass() override;

            virtual void BindShaderVariables(Uint32 index, ShaderVariables* pVariables) override;

            virtual void BindVertexBuffers(Uint32 startIndex, Uint32 numBuffers, VertexBuffer** ppBuffers, Uint32* pOffsets) override;
            virtual void BindIndexBuffer(IndexBuffer* pBuffer, Uint32 offset) override;

            virtual void Draw(Uint32 numVertices, Uint32 baseVertex, Uint32 numInstances, Uint32 baseInstance) override;
            virtual void DrawIndexed(Uint32 numIndices, Uint32 baseIndex, Uint32 baseVertex, Uint32 numInstances, Uint32 baseInstance) override;

            virtual void ExecuteSubCommands(Uint32 numCommandLists, CommandList** ppCommandLists) override;

            virtual void SetPipelineState(ComputePipelineState* pipelineState) override;
            virtual void Dispatch(Uint32 groupX, Uint32 groupY, Uint32 groupZ) override;

            virtual void Submit() override;

        private:
            VulkanDevice& mDevice;
            VulkanCommandPoolManager& mPoolManager;

            VulkanCommandBuffer mCommandBuffer;

            CommandListType mType;
            Uint32 mPoolIndex;
            bool mIsSub;

            GraphicsPipelineStateVk* mpBoundGraphicsPipelineState;
            ComputePipelineStateVk* mpBoundComputePipelineState;
            RenderPassVk* mpBoundRenderPass;
        };
    } // namespace cube
} // namespace rapi
