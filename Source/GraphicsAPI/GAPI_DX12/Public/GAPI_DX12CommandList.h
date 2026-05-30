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

            void SetViewports(ConstArrayView<Viewport> viewports) override;
            void SetScissors(ConstArrayView<ScissorRect> scissors) override;
            void SetPrimitiveTopology(PrimitiveTopology primitiveTopology) override;

            void SetGraphicsPipeline(SharedPtr<GraphicsPipeline> graphicsPipeline) override;

            virtual void BeginRenderPass(ArrayView<const ColorAttachment> colors, DepthStencilAttachment depthStencil) override;
            virtual void EndRenderPass() override;

            void BindIndexBuffer(SharedPtr<Buffer> buffer, Uint32 offset) override;

            void Draw(Uint32 numVertices, Uint32 baseVertex, Uint32 numInstances, Uint32 baseInstance) override;
            void DrawIndexed(Uint32 numIndices, Uint32 baseIndex, Uint32 baseVertex, Uint32 numInstances, Uint32 baseInstance) override;

            virtual void SetConstantBuffer(Uint32 index, SharedPtr<BufferSRV> constantBuffer) override;
            virtual void UseResource(SharedPtr<BufferSRV> srv) override;
            virtual void UseResource(SharedPtr<BufferUAV> uav) override;
            virtual void UseResource(SharedPtr<TextureSRV> srv) override;
            virtual void UseResource(SharedPtr<TextureUAV> uav) override;

            void ResourceTransition(TransitionState state) override;
            void ResourceTransition(ArrayView<const TransitionState> states) override;

            void SetComputePipeline(SharedPtr<ComputePipeline> computePipeline) override;
            virtual void DispatchThreads(Uint32 numThreadsX, Uint32 numThreadsY, Uint32 numThreadsZ) override;

            virtual void CopyTexture(SharedPtr<Texture> srcTexture, SharedPtr<Texture> dstTexture) override;

            virtual void BeginTimestamp(StringView name) override;
            virtual void EndTimestamp() override;

            void Submit(bool waitUntilFinished) override;

            bool IsWriting() const { return mState == State::Writing; }
            bool IsInRenderPass() const { return mIsInRenderPass; }

        private:
            void ProcessBeforeEnd();

            enum class State
            {
                Initial,
                Writing,
                Closed
            };

            DX12Device& mDevice;

            ComPtr<ID3D12GraphicsCommandList> mCommandList;
            State mState;

            Vector<SharedPtr<DX12APIObject>> mBoundObjects;

            Uint32 mComputeThreadGroupSizeX;
            Uint32 mComputeThreadGroupSizeY;
            Uint32 mComputeThreadGroupSizeZ;

            bool mIsInRenderPass = false;
            bool mHasQuery = false;

            struct TimestampBegin
            {
                String name;
                Uint32 beginQueryIndex;
            };
            Vector<TimestampBegin> mTimestampStack;

            Vector<AnsiString> mCurrentEventNameList;
        };
    } // namespace gapi
} // namespace cube
