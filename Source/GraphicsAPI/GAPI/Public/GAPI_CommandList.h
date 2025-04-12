#pragma once

#include "GAPIHeader.h"

#include "Vector.h"

#include "GAPI_Resource.h"

namespace cube
{
    namespace gapi
    {
        class ShaderVariablesLayout;
        class Buffer;
        class Pipeline;
        class ShaderVariables;
        class Viewport;

        struct ScissorRect
        {
            Int32 x;
            Int32 y;
            Uint32 width;
            Uint32 height;
        };

        enum class PrimitiveTopology
        {
            PointList,
            LineList,
            LineStrip,
            TriangleList,
            TriangleStrip
        };

        struct CommandListCreateInfo
        {
            const char* debugName = "Unknown";
        };

        class CommandList
        {
        public:
            CommandList() = default;
            virtual ~CommandList() = default;

            virtual void Begin() = 0;
            virtual void End() = 0;
            virtual void Reset() = 0;

            virtual void SetViewports(Uint32 numViewports, const SharedPtr<Viewport>* pViewports) = 0;
            virtual void SetScissors(Uint32 numScissors, const ScissorRect* pScissors) = 0;
            virtual void SetPrimitiveTopology(PrimitiveTopology primitiveTopology) = 0;

            virtual void SetShaderVariablesLayout(SharedPtr<ShaderVariablesLayout> shaderVariablesLayout) = 0;
            virtual void SetGraphicsPipeline(SharedPtr<Pipeline> graphicsPipeline) = 0;
            virtual void SetRenderTarget(SharedPtr<Viewport> viewport) = 0;
            virtual void ClearRenderTargetView(SharedPtr<Viewport> viewport, Float4 color) = 0;
            virtual void ClearDepthStencilView(SharedPtr<Viewport> viewport, float depth) = 0;
            virtual void SetShaderVariableConstantBuffer(Uint32 index, SharedPtr<Buffer> constantBuffer) = 0;

            virtual void ResourceTransition(SharedPtr<Buffer> buffer, ResourceStateFlags srcState, ResourceStateFlags dstState) = 0;
            virtual void ResourceTransition(SharedPtr<Viewport> viewport, ResourceStateFlags srcState, ResourceStateFlags dstState) = 0;

            virtual void BindVertexBuffers(Uint32 startIndex, Uint32 numBuffers, const SharedPtr<Buffer>* pBuffers, Uint32* pOffsets) = 0;
            virtual void BindIndexBuffer(SharedPtr<Buffer> buffer, Uint32 offset) = 0;

            virtual void Draw(Uint32 numVertices, Uint32 baseVertex, Uint32 numInstances = 1, Uint32 baseInstance = 0) = 0;
            virtual void DrawIndexed(Uint32 numIndices, Uint32 baseIndex, Uint32 baseVertex, Uint32 numInstances = 1, Uint32 baseInstance = 0) = 0;

            virtual void InsertTimestamp(const String& name) = 0;

            virtual void Submit() = 0;
        };
    } // namespace gapi
} // namespace cube
