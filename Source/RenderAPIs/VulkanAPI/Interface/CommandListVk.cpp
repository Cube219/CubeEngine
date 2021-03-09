#include "CommandListVk.h"

#include "../VulkanDevice.h"
#include "BufferVk.h"
#include "FramebufferVk.h"
#include "PipelineStateVk.h"
#include "RenderPassVk.h"
#include "ShaderVariablesVk.h"

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
            mpBoundGraphicsPipelineState(nullptr),
            mpBoundComputePipelineState(nullptr),
            mpBoundRenderPass(nullptr)
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

        void CommandListVk::BeginSubWithRenderPass(RenderPass* pRenderPass, Framebuffer* pFramebuffer, Uint32 subpassIndex)
        {
            CHECK(mIsSub == true, "Only sub command list can call BeginSubWithRenderPass.");
            CHECK(mType == CommandListType::Graphics, "Only graphics command list can call BeginSubWithRenderPass")

            VkResult res;

            VkCommandBufferInheritanceInfo inheritance;
            inheritance.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
            inheritance.pNext = nullptr;
            inheritance.renderPass = DCast(RenderPassVk*)(pRenderPass)->GetVkRenderPass();
            inheritance.subpass = subpassIndex;
            inheritance.framebuffer = DCast(FramebufferVk*)(pFramebuffer)->GetHandle();
            inheritance.occlusionQueryEnable = VK_FALSE;
            inheritance.queryFlags = 0;
            inheritance.pipelineStatistics = 0;

            VkCommandBufferBeginInfo beginInfo;
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.pNext = nullptr;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
            beginInfo.pInheritanceInfo = &inheritance;

            res = vkBeginCommandBuffer(mCommandBuffer.handle, &beginInfo);
            CHECK_VK(res, "Failed to begin the secondary command buffer.");
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

            mpBoundRenderPass = nullptr;
            mpBoundGraphicsPipelineState = nullptr;
            mpBoundComputePipelineState = nullptr;
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

            mpBoundGraphicsPipelineState = DCast(GraphicsPipelineStateVk*)(pipelineState);
            vkCmdBindPipeline(mCommandBuffer.handle, VK_PIPELINE_BIND_POINT_GRAPHICS, mpBoundGraphicsPipelineState->GetHandle());
        }

        void CommandListVk::BeginRenderPass(RenderPass* pRenderPass, Framebuffer* pFramebuffer, Uint32 numClearValues, ClearValue* pClearValues, bool useSubCommandList)
        {
            CHECK(mType == CommandListType::Graphics, "Only graphics command list can set render pass.");
            CHECK(mIsSub == false, "Only main command list can set render pass.");
            CHECK(mpBoundRenderPass == nullptr, "The render pass is currently bound.");

            FramebufferVk* pVkFramebuffer = DCast(FramebufferVk*)(pFramebuffer);
            mpBoundRenderPass = DCast(RenderPassVk*)(pRenderPass);

            FrameVector<VkClearValue> clearValues(numClearValues);
            for(Uint32 i = 0; i < numClearValues; ++i) {
                if(pClearValues[i].useColor == true) {
                    float r = pClearValues[i].color.value[0];
                    float g = pClearValues[i].color.value[1];
                    float b = pClearValues[i].color.value[2];
                    float a = pClearValues[i].color.value[3];
                    clearValues[i].color.float32[0] = r;
                    clearValues[i].color.float32[1] = g;
                    clearValues[i].color.float32[2] = b;
                    clearValues[i].color.float32[3] = a;
                } else {
                    clearValues[i].depthStencil.depth = pClearValues[i].depthStencil.depth;
                    clearValues[i].depthStencil.stencil = pClearValues[i].depthStencil.stencil;
                }
            }

            VkRenderPassBeginInfo beginInfo;
            beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            beginInfo.pNext = nullptr;
            beginInfo.renderPass = mpBoundRenderPass->GetVkRenderPass();
            beginInfo.framebuffer = pVkFramebuffer->GetHandle();
            beginInfo.renderArea = { {0, 0}, { pVkFramebuffer->GetWidth(), pVkFramebuffer->GetHeight() } };
            beginInfo.clearValueCount = numClearValues;
            beginInfo.pClearValues = clearValues.data();

            if(useSubCommandList == false) {
                vkCmdBeginRenderPass(mCommandBuffer.handle, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
            } else {
                vkCmdBeginRenderPass(mCommandBuffer.handle, &beginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
            }
        }

        void CommandListVk::NextSubpass(bool useSubCommandList)
        {
            CHECK(mType == CommandListType::Graphics, "Only graphics command list can move next subpass.");
            CHECK(mIsSub == false, "Only main command list can move next subpass.");
            CHECK(mpBoundRenderPass != nullptr, "The render pass is not bound.");

            if(useSubCommandList == false) {
                vkCmdNextSubpass(mCommandBuffer.handle, VK_SUBPASS_CONTENTS_INLINE);
            } else {
                vkCmdNextSubpass(mCommandBuffer.handle, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
            }
        }

        void CommandListVk::EndRenderPass()
        {
            CHECK(mType == CommandListType::Graphics, "Only graphics command list can end render pass.");
            CHECK(mIsSub == false, "Only main command list can end render pass.");
            CHECK(mpBoundRenderPass != nullptr, "The render pass is not bound.");

            vkCmdEndRenderPass(mCommandBuffer.handle);

            mpBoundRenderPass = nullptr;
        }

        void CommandListVk::BindShaderVariables(Uint32 index, ShaderVariables* pVariables)
        {
            VkPipelineBindPoint bindPoint;
            VkPipelineLayout layout = VK_NULL_HANDLE;
            switch(mType) {
                case CommandListType::Graphics:
                    bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
                    layout = mpBoundGraphicsPipelineState->GetPipelineLayout();
                    break;
                case CommandListType::Compute:
                    bindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;
                    layout = mpBoundComputePipelineState->GetPipelineLayout();
                    break;
                case CommandListType::Transfer:
                    ASSERTION_FAILED("Only graphics and compute command list can bind shader parameters.");
                    break;
            }

            DCast(ShaderVariablesVk*)(pVariables)->BIndVariables(mCommandBuffer.handle, bindPoint, layout, index);
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

            mpBoundComputePipelineState = DCast(ComputePipelineStateVk*)(pipelineState);
            vkCmdBindPipeline(mCommandBuffer.handle, VK_PIPELINE_BIND_POINT_COMPUTE, mpBoundComputePipelineState->GetHandle());
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
