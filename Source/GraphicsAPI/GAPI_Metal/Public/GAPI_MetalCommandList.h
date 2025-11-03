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

            virtual void BeginEvent(StringView name) override {}
            virtual void EndEvent() override {}

            void SetViewports(ArrayView<Viewport> viewports) override {}
            void SetScissors(ArrayView<ScissorRect> scissors) override {}
            void SetPrimitiveTopology(PrimitiveTopology primitiveTopology) override {}

            void SetGraphicsPipeline(SharedPtr<GraphicsPipeline> graphicsPipeline) override {}
            virtual void SetRenderTargets(ArrayView<ColorAttachment> colors, DepthStencilAttachment depthStencil) override {}

            void BindVertexBuffers(Uint32 startIndex, ArrayView<SharedPtr<Buffer>> buffers, ArrayView<Uint32> offsets) override {};
            void BindIndexBuffer(SharedPtr<Buffer> buffer, Uint32 offset) override {};

            void Draw(Uint32 numVertices, Uint32 baseVertex, Uint32 numInstances = 1, Uint32 baseInstance = 0) override {};
            void DrawIndexed(Uint32 numIndices, Uint32 baseIndex, Uint32 baseVertex, Uint32 numInstances = 1, Uint32 baseInstance = 0) override {};

            void SetShaderVariablesLayout(SharedPtr<ShaderVariablesLayout> shaderVariablesLayout) override {};
            void SetShaderVariableConstantBuffer(Uint32 index, SharedPtr<Buffer> constantBuffer) override {};
            void BindTexture(SharedPtr<Texture> texture) override {};
            void BindSampler(SharedPtr<Sampler> sampler) override {};

            void ResourceTransition(TransitionState state) override {};
            void ResourceTransition(ArrayView<TransitionState> states) override {};

            void SetComputePipeline(SharedPtr<ComputePipeline> computePipeline) override {};
            void Dispatch(Uint32 threadGroupX, Uint32 threadGroupY, Uint32 threadGroupZ) override {};

            void InsertTimestamp(const String& name) override {};

            void Submit() override {};
        };
    } // namespace gapi
} // namespace cube
