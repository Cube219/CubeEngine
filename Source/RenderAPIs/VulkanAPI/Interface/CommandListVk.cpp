#include "CommandListVk.h"

#include "../VulkanDevice.h"

namespace cube
{
    namespace rapi
    {
        CommandListVk::CommandListVk(VulkanDevice& device, VulkanCommandPoolManager& poolManager, VulkanCommandBuffer commandBuffer) :
            mDevice(device),
            mPoolManager(poolManager),
            mCommandBuffer(commandBuffer)
        {
            
        }

        CommandListVk::~CommandListVk()
        {
            if(mCommandBuffer.handle != VK_NULL_HANDLE) {
                mPoolManager.FreeCommandBuffer(mCommandBuffer);
            }
        }

        void CommandListVk::Begin() {}

        void CommandListVk::End() {}

        void CommandListVk::Reset() {}

        void CommandListVk::SetViewports(Uint32 numViewports, const Viewport* pViewports) {}

        void CommandListVk::SetScissors(Uint32 numScissors, const Rect2D* pScissors) {}

        void CommandListVk::SetPipelineState(GraphicsPipelineState* pipelineState) {}

        void CommandListVk::BeginRenderPass(RenderPass* pRenderPass) {}

        void CommandListVk::NextSubpass() {}

        void CommandListVk::EndRenderPass() {}

        void CommandListVk::BindShaderVariables(Uint32 index, ShaderVariables* pVariables) {}

        void CommandListVk::BindVertexBuffers(Uint32 startIndex, Uint32 numBuffers, VertexBuffer** ppBuffers, Uint32* pOffsets) {}

        void CommandListVk::BindIndexBuffer(IndexBuffer* pBuffer, Uint32 offset) {}

        void CommandListVk::Draw(Uint32 numVertices, Uint32 baseVertex, Uint32 numInstances, Uint32 baseInstance) {}

        void CommandListVk::DrawIndexed(Uint32 numIndices, Uint32 baseIndex, Uint32 baseVertex, Uint32 numInstances, Uint32 baseInstance) {}

        void CommandListVk::SetPipelineState(ComputePipelineState* pipelineState) {}

        void CommandListVk::Dispatch(Uint32 groupX, Uint32 groupY, Uint32 groupZ) {}

        void CommandListVk::Submit() {}
    } // namespace rapi
} // namespace cube
