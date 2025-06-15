#pragma once

#include "MetalHeader.h"

#include "GAPI_CommandList.h"

namespace cube
{
    namespace gapi
    {
        class MetalCommandList : public CommandList
        {
        public:
            MetalCommandList(const CommandListCreateInfo& info) {}
            virtual ~MetalCommandList() {}

            void Begin() override {}
            void End() override {}
            void Reset() override {}

            void SetViewports(ArrayView<SharedPtr<Viewport>> viewports) override {}
            void SetScissors(ArrayView<ScissorRect> scissors) override {}
            void SetPrimitiveTopology(PrimitiveTopology primitiveTopology) override {}

            void SetShaderVariablesLayout(SharedPtr<ShaderVariablesLayout> shaderVariablesLayout) override {}
            void SetGraphicsPipeline(SharedPtr<Pipeline> graphicsPipeline) override {}
            void SetRenderTarget(SharedPtr<Viewport> viewport) override {}
            void ClearRenderTargetView(SharedPtr<Viewport> viewport, Float4 color) override {}
            void ClearDepthStencilView(SharedPtr<Viewport> viewport, float depth) override {}
            void SetShaderVariableConstantBuffer(Uint32 index, SharedPtr<Buffer> constantBuffer) override {}
            void BindTexture(SharedPtr<Texture> texture) override {}
            void BindSampler(SharedPtr<Sampler> sampler) override {}

            void ResourceTransition(SharedPtr<Buffer> buffer, ResourceStateFlags srcState, ResourceStateFlags dstState) override {}
            void ResourceTransition(SharedPtr<Viewport> viewport, ResourceStateFlags srcState, ResourceStateFlags dstState) override {}

            void BindVertexBuffers(Uint32 startIndex, ArrayView<SharedPtr<Buffer>> buffers, ArrayView<Uint32> offsets) override {}
            void BindIndexBuffer(SharedPtr<Buffer> buffer, Uint32 offset) override {}

            void Draw(Uint32 numVertices, Uint32 baseVertex, Uint32 numInstances, Uint32 baseInstance) override {}
            void DrawIndexed(Uint32 numIndices, Uint32 baseIndex, Uint32 baseVertex, Uint32 numInstances, Uint32 baseInstance) override {}

            void InsertTimestamp(const String& name) override {}

            void Submit() override {}
        };
    } // namespace gapi
} // namespace cube
