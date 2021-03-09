#pragma once

#include "../RenderAPIHeader.h"

namespace cube
{
    namespace rapi
    {
        struct Viewport
        {
            float x;
            float y;
            float width;
            float height;
            float minDepth;
            float maxDepth;
        };

        struct Rect2D
        {
            Int32 x;
            Int32 y;
            Uint32 width;
            Uint32 height;
        };

        enum class CommandListType
        {
            Graphics,
            Transfer,
            Compute
        };

        struct CommandListAllocateInfo
        {
            CommandListType type = CommandListType::Graphics;
            Uint32 poolIndex = 0;
            bool isSub = false;

            const char* debugName = "";
        };

        class CommandList
        {
        public:
            CommandList() = default;
            virtual ~CommandList() = default;

            virtual void Begin() = 0;
            virtual void BeginSubWithRenderPass(RenderPass* pRenderPass, Framebuffer* pFramebuffer, Uint32 subpassIndex = 0) = 0;
            virtual void End() = 0;
            virtual void Reset() = 0;

            virtual void SetViewports(Uint32 numViewports, const Viewport* pViewports) = 0;
            virtual void SetScissors(Uint32 numScissors, const Rect2D* pScissors) = 0;

            virtual void SetPipelineState(GraphicsPipelineState* pipelineState) = 0;

            virtual void BeginRenderPass(RenderPass* pRenderPass, Framebuffer* pFramebuffer, Uint32 numClearValues, ClearValue* pClearValues, bool useSubCommandList) = 0;
            virtual void NextSubpass(bool useSubCommandList) = 0;
            virtual void EndRenderPass() = 0;

            virtual void BindShaderVariables(Uint32 index, ShaderVariables* pVariables) = 0;

            virtual void BindVertexBuffers(Uint32 startIndex, Uint32 numBuffers, VertexBuffer** ppBuffers, Uint32* pOffsets) = 0;
            virtual void BindIndexBuffer(IndexBuffer* pBuffer, Uint32 offset) = 0;

            virtual void Draw(Uint32 numVertices, Uint32 baseVertex, Uint32 numInstances, Uint32 baseInstance) = 0;
            virtual void DrawIndexed(Uint32 numIndices, Uint32 baseIndex, Uint32 baseVertex, Uint32 numInstances, Uint32 baseInstance) = 0;

            virtual void ExecuteSubCommands(Uint32 numCommandLists, CommandList** ppCommandLists) = 0;

            virtual void SetPipelineState(ComputePipelineState* pipelineState) = 0;
            virtual void Dispatch(Uint32 groupX, Uint32 groupY, Uint32 groupZ) = 0;

            virtual void Submit() = 0;
        };
    } // namespace rapi
} // namespace cube
