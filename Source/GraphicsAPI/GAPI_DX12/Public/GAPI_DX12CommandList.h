#pragma once

#include "DX12Header.h"

#include "GAPI_CommandList.h"

#include "DX12APIObject.h"

namespace cube
{
    class DX12CommandListManager;
    class DX12Device;
    class DX12QueryManager;
    class DX12QueueManager;

    namespace gapi
    {
        class Buffer;
        class Pipeline;
        class Viewport;

        class DX12CommandList : public CommandList, public DX12APIObject
        {
        public:
            DX12CommandList(DX12Device& device, const CommandListCreateInfo& info);
            virtual ~DX12CommandList();

            void Begin() override;
            void End() override;
            void Reset() override;

            void SetViewports(ArrayView<SharedPtr<Viewport>> viewports) override;
            void SetScissors(ArrayView<ScissorRect> scissors) override;
            void SetPrimitiveTopology(PrimitiveTopology primitiveTopology) override;

            void SetShaderVariablesLayout(SharedPtr<ShaderVariablesLayout> shaderVariablesLayout) override;
            void SetGraphicsPipeline(SharedPtr<Pipeline> graphicsPipeline) override;
            void SetRenderTarget(SharedPtr<Viewport> viewport) override;
            void ClearRenderTargetView(SharedPtr<Viewport> viewport, Float4 color) override;
            void ClearDepthStencilView(SharedPtr<Viewport> viewport, float depth) override;
            void SetShaderVariableConstantBuffer(Uint32 index, SharedPtr<Buffer> constantBuffer) override;

            void ResourceTransition(SharedPtr<Buffer> buffer, ResourceStateFlags srcState, ResourceStateFlags dstState) override;
            void ResourceTransition(SharedPtr<Viewport> viewport, ResourceStateFlags srcState, ResourceStateFlags dstState) override;

            void BindVertexBuffers(Uint32 startIndex, ArrayView<SharedPtr<Buffer>> buffers, ArrayView<Uint32> offsets) override;
            void BindIndexBuffer(SharedPtr<Buffer> buffer, Uint32 offset) override;

            void Draw(Uint32 numVertices, Uint32 baseVertex, Uint32 numInstances, Uint32 baseInstance) override;
            void DrawIndexed(Uint32 numIndices, Uint32 baseIndex, Uint32 baseVertex, Uint32 numInstances, Uint32 baseInstance) override;

            void InsertTimestamp(const String& name) override;

            void Submit() override;

        private:
            enum class State
            {
                Initial,
                Writing,
                Closed
            };

            DX12CommandListManager& mCommandListManager;
            DX12QueueManager& mQueueManager;
            DX12QueryManager& mQueryManager;

            ComPtr<ID3D12GraphicsCommandList> mCommandList;
            State mState;

            bool hasTimestampQuery = false;

            Vector<SharedPtr<DX12APIObject>> mBoundObjects;
        };
    } // namespace gapi
} // namespace cube
