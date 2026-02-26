#pragma once

#include "DX12Header.h"

#include "DX12APIObject.h"
#include "GAPI_CommandList.h"

namespace cube
{
    class DX12CommandListManager;
    class DX12DescriptorManager;
    class DX12Device;
    class DX12QueryManager;
    class DX12QueueManager;

    namespace gapi
    {
        class Buffer;
        class ComputePipeline;
        class DX12ShaderParameterHelper;
        class GraphicsPipeline;
        class Sampler;
        class Texture;

        class DX12CommandList : public CommandList, public DX12APIObject
        {
        public:
            DX12CommandList(DX12Device& device, const CommandListCreateInfo& info);
            virtual ~DX12CommandList();

            void Begin() override;
            void End() override;
            void Reset() override;

            virtual void BeginEvent(StringView name) override;
            virtual void EndEvent() override;

            void SetViewports(ArrayView<Viewport> viewports) override;
            void SetScissors(ArrayView<ScissorRect> scissors) override;
            void SetPrimitiveTopology(PrimitiveTopology primitiveTopology) override;

            void SetGraphicsPipeline(SharedPtr<GraphicsPipeline> graphicsPipeline) override;

            virtual void BeginRenderPass(ArrayView<const ColorAttachment> colors, DepthStencilAttachment depthStencil) override;
            virtual void EndRenderPass() override;

            void BindVertexBuffers(Uint32 startIndex, ArrayView<SharedPtr<Buffer>> buffers, ArrayView<Uint32> offsets) override;
            void BindIndexBuffer(SharedPtr<Buffer> buffer, Uint32 offset) override;

            void Draw(Uint32 numVertices, Uint32 baseVertex, Uint32 numInstances, Uint32 baseInstance) override;
            void DrawIndexed(Uint32 numIndices, Uint32 baseIndex, Uint32 baseVertex, Uint32 numInstances, Uint32 baseInstance) override;

            void SetShaderVariableConstantBuffer(Uint32 index, SharedPtr<Buffer> constantBuffer) override;
            virtual void UseResource(SharedPtr<TextureSRV> srv) override;
            virtual void UseResource(SharedPtr<TextureUAV> uav) override;

            void ResourceTransition(TransitionState state) override;
            void ResourceTransition(ArrayView<TransitionState> states) override;

            void SetComputePipeline(SharedPtr<ComputePipeline> computePipeline) override;
            virtual void DispatchThreads(Uint32 numThreadsX, Uint32 numThreadsY, Uint32 numThreadsZ) override;

            void InsertTimestamp(const String& name) override;

            void Submit() override;

            bool IsWriting() const { return mState == State::Writing; }
            bool IsInRenderPass() const { return mIsInRenderPass; }

        private:
            enum class State
            {
                Initial,
                Writing,
                Closed
            };

            DX12CommandListManager& mCommandListManager;
            DX12DescriptorManager& mDescriptorManager;
            DX12QueueManager& mQueueManager;
            DX12QueryManager& mQueryManager;
            DX12ShaderParameterHelper& mShaderParameterHelper;

            ComPtr<ID3D12GraphicsCommandList> mCommandList;
            State mState;

            Vector<SharedPtr<DX12APIObject>> mBoundObjects;

            Uint32 mComputeThreadGroupSizeX;
            Uint32 mComputeThreadGroupSizeY;
            Uint32 mComputeThreadGroupSizeZ;

            bool mIsInRenderPass = false;
            bool mHasTimestampQuery = false;

            Vector<AnsiString> mCurrentEventNameList;
        };
    } // namespace gapi
} // namespace cube
