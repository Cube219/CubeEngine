#include "CommandListVk.h"

#include "../VulkanDevice.h"
#include "BufferVk.h"
#include "PipelineStateVk.h"

#include "Core/Allocator/FrameAllocator.h"
#include "Core/Assertion.h"

namespace cube
{
    namespace rapi
    {
        CommandListVk::CommandListVk(VulkanDevice& device, VulkanCommandPoolManager& poolManager, const CommandListAllocateInfo& info, VulkanCommandBuffer commandBuffer) :
            mDevice(device),
            mPoolManager(poolManager),
            mCommandBuffer(commandBuffer),
            mType(info.type),
            mPoolIndex(info.poolIndex),
            mIsSub(info.isSub),
            mpBindedGraphicsPipelineState(nullptr),
            mpBindedComputePipelineState(nullptr),
            mpBindedRenderPass(nullptr)
        {
            
        }

        CommandListVk::~CommandListVk()
        {
            if(mCommandBuffer.handle != VK_NULL_HANDLE) {
                mPoolManager.FreeCommandBuffer(mCommandBuffer);
            }
        }

        void CommandListVk::Begin()
        {
            VkResult res;

            VkCommandBufferBeginInfo beginInfo;
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.pNext = nullptr;
            beginInfo.flags = 0;
            beginInfo.pInheritanceInfo = nullptr;

            res = vkBeginCommandBuffer(mCommandBuffer.handle, &beginInfo);
            CHECK_VK(res, "Failed to begin the command buffer.");
        }

        void CommandListVk::End()
        {
            VkResult res;

            res = vkEndCommandBuffer(mCommandBuffer.handle);
            CHECK_VK(res, "Failed to end the command buffer.");
        }

        void CommandListVk::Reset()
        {
            VkResult res;

            res = vkResetCommandBuffer(mCommandBuffer.handle, 0);
            CHECK_VK(res, "Failed to reset command buffer.");

            mpBindedRenderPass = nullptr;
            mpBindedGraphicsPipelineState = nullptr;
            mpBindedComputePipelineState = nullptr;
        }

        void CommandListVk::SetViewports(Uint32 numViewports, const Viewport* pViewports)
        {
            CHECK(mType == CommandListType::Graphics, "Only graphics command list can set viewports.");

            FrameVector<VkViewport> vp(numViewports);
            for(Uint32 i = 0; i < numViewports; ++i) {
                vp[i].x = pViewports[i].x;
                vp[i].y = pViewports[i].y;
                vp[i].width = pViewports[i].width;
                vp[i].height = pViewports[i].height;
                vp[i].minDepth = pViewports[i].minDepth;
                vp[i].maxDepth = pViewports[i].maxDepth;
            }

            vkCmdSetViewport(mCommandBuffer.handle, 0, numViewports, vp.data());
        }

        void CommandListVk::SetScissors(Uint32 numScissors, const Rect2D* pScissors)
        {
            CHECK(mType == CommandListType::Graphics, "Only graphics command list can set scissors.");

            FrameVector<VkRect2D> sc(numScissors);
            for(Uint32 i = 0; i < numScissors; i++) {
                sc[i].offset.x = pScissors[i].x;
                sc[i].offset.y = pScissors[i].y;
                sc[i].extent.width = pScissors[i].width;
                sc[i].extent.height = pScissors[i].height;
            }

            vkCmdSetScissor(mCommandBuffer.handle, 0, numScissors, sc.data());
        }

        void CommandListVk::SetPipelineState(GraphicsPipelineState* pipelineState)
        {
            CHECK(mType == CommandListType::Graphics, "Only graphics command list can set graphics pipeline state.");

            mpBindedGraphicsPipelineState = DCast(GraphicsPipelineStateVk*)(pipelineState);
            vkCmdBindPipeline(mCommandBuffer.handle, VK_PIPELINE_BIND_POINT_GRAPHICS, mpBindedGraphicsPipelineState->GetHandle());
        }

        void CommandListVk::BeginRenderPass(RenderPass* pRenderPass)
        {
            
        }

        void CommandListVk::NextSubpass()
        {
            
        }

        void CommandListVk::EndRenderPass()
        {
            
        }

        void CommandListVk::BindShaderVariables(Uint32 index, ShaderVariables* pVariables)
        {
            
        }

        void CommandListVk::BindVertexBuffers(Uint32 startIndex, Uint32 numBuffers, VertexBuffer** ppBuffers, Uint32* pOffsets)
        {
            CHECK(mType == CommandListType::Graphics, "Only graphics command list can bind vertex buffers.");

            FrameVector<VkBuffer> bufs(numBuffers);
            FrameVector<Uint64> offsets(numBuffers);
            for(Uint32 i = 0; i < numBuffers; i++) {
                bufs[i] = DCast(VertexBufferVk*)(ppBuffers[i])->GetHandle();
                offsets[i] = pOffsets[i];
            }

            vkCmdBindVertexBuffers(mCommandBuffer.handle, startIndex, numBuffers, bufs.data(), offsets.data());
        }

        void CommandListVk::BindIndexBuffer(IndexBuffer* pBuffer, Uint32 offset)
        {
            CHECK(mType == CommandListType::Graphics, "Only graphics command list can bind index buffers.");

            IndexBufferVk* buf = DCast(IndexBufferVk*)(pBuffer);
            vkCmdBindIndexBuffer(mCommandBuffer.handle, buf->GetHandle(), offset, buf->GetIndexType());
        }

        void CommandListVk::Draw(Uint32 numVertices, Uint32 baseVertex, Uint32 numInstances, Uint32 baseInstance)
        {
            CHECK(mType == CommandListType::Graphics, "Only graphics command list can draw.");

            vkCmdDraw(mCommandBuffer.handle, numVertices, numInstances, baseVertex, baseInstance);
        }

        void CommandListVk::DrawIndexed(Uint32 numIndices, Uint32 baseIndex, Uint32 baseVertex, Uint32 numInstances, Uint32 baseInstance)
        {
            CHECK(mType == CommandListType::Graphics, "Only graphics command list can draw indexed.");

            vkCmdDrawIndexed(mCommandBuffer.handle, numIndices, numInstances, baseIndex, baseVertex, baseInstance);
        }

        void CommandListVk::ExecuteSubCommands(Uint32 numCommandLists, CommandList** ppCommandLists)
        {
            FrameVector<VkCommandBuffer> cmds(numCommandLists);
            for(Uint32 i = 0; i < numCommandLists; i++) {
                CommandListVk* vkCmd = DCast(CommandListVk*)(ppCommandLists[i]);
                CHECK(vkCmd->mIsSub == true, "Only the sub-command list can be executed.");
                cmds[i] = vkCmd->GetVkCommandBuffer();
            }

            vkCmdExecuteCommands(mCommandBuffer.handle, numCommandLists, cmds.data());
        }

        void CommandListVk::SetPipelineState(ComputePipelineState* pipelineState)
        {
            CHECK(mType == CommandListType::Compute, "Only compute command list can set compute pipeline state.");

            mpBindedComputePipelineState = DCast(ComputePipelineStateVk*)(pipelineState);
            vkCmdBindPipeline(mCommandBuffer.handle, VK_PIPELINE_BIND_POINT_COMPUTE, mpBindedComputePipelineState->GetHandle());
        }

        void CommandListVk::Dispatch(Uint32 groupX, Uint32 groupY, Uint32 groupZ)
        {
            CHECK(mType == CommandListType::Compute, "Only compute command list can dispatch compute shaders.");

            vkCmdDispatch(mCommandBuffer.handle, groupX, groupY, groupZ);
        }

        void CommandListVk::Submit()
        {
            
        }
    } // namespace rapi
} // namespace cube
