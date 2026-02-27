#pragma once

#include "MetalHeader.h"

#include "GAPI_CommandList.h"
#include "MetalTimestampManager.h"

namespace cube
{
    class MetalDevice;

    namespace gapi
    {
        struct MetalEncoderState
        {
            Vector<MTLViewport> viewports;
            Vector<MTLScissorRect> scissors;
            MTLPrimitiveType primitiveType;

            struct ConstantBuffer
            {
                id<MTLBuffer> buffer;
                bool isSet = false;
            };
            Map<int, ConstantBuffer> constantBuffers;
            void Clear()
            {
                viewports.clear();
                scissors.clear();
                primitiveType = MTLPrimitiveTypeTriangle;
                constantBuffers.clear();
            }

            void SetViewports(ArrayView<Viewport> newViewports);
            void ApplyViewports(id<MTLRenderCommandEncoder> encoder);

            void SetScissors(ArrayView<ScissorRect> newScissors);
            void ApplyScissors(id<MTLRenderCommandEncoder> encoder);

            void SetPrimitiveTopology(PrimitiveTopology newPrimitiveTopology);

            void SetConstantBuffer(SharedPtr<Buffer> buffer, Uint32 index);
            void ApplyConstantBuffers(id<MTLRenderCommandEncoder> encoder, bool forceAll = false);
            void ApplyConstantBuffers(id<MTLComputeCommandEncoder> computeEncoder, bool forceAll = false);

            void ApplyAll(id<MTLRenderCommandEncoder> encoder);
            void ApplyAll(id<MTLComputeCommandEncoder> encoder);
        };

        class MetalCommandList : public CommandList
        {
        public:
            MetalCommandList(const CommandListCreateInfo& info, MetalDevice& device);
            virtual ~MetalCommandList();

            virtual void Begin() override;
            virtual void End() override;
            virtual void Reset() override;

            virtual void BeginEvent(StringView name) override {}
            virtual void EndEvent() override {}

            virtual void SetViewports(ArrayView<Viewport> viewports) override;
            virtual void SetScissors(ArrayView<ScissorRect> scissors) override;
            virtual void SetPrimitiveTopology(PrimitiveTopology primitiveTopology) override;

            virtual void SetGraphicsPipeline(SharedPtr<GraphicsPipeline> graphicsPipeline) override;

            virtual void BeginRenderPass(ArrayView<const ColorAttachment> colors, DepthStencilAttachment depthStencil) override;
            virtual void EndRenderPass() override;

            virtual void BindVertexBuffers(Uint32 startIndex, ArrayView<SharedPtr<Buffer>> buffers, ArrayView<Uint32> offsets) override;
            virtual void BindIndexBuffer(SharedPtr<Buffer> buffer, Uint32 offset) override;

            virtual void Draw(Uint32 numVertices, Uint32 baseVertex, Uint32 numInstances, Uint32 baseInstance) override;
            virtual void DrawIndexed(Uint32 numIndices, Uint32 baseIndex, Uint32 baseVertex, Uint32 numInstances, Uint32 baseInstance) override;

            virtual void SetShaderVariableConstantBuffer(Uint32 index, SharedPtr<Buffer> constantBuffer) override;
            virtual void UseResource(SharedPtr<TextureSRV> srv) override;
            virtual void UseResource(SharedPtr<TextureUAV> uav) override;

            virtual void ResourceTransition(TransitionState state) override;
            virtual void ResourceTransition(ArrayView<const TransitionState> states) override;

            virtual void SetComputePipeline(SharedPtr<ComputePipeline> computePipeline) override;
            virtual void DispatchThreads(Uint32 numThreadsX, Uint32 numThreadsY, Uint32 numThreadsZ) override;

            virtual void InsertTimestamp(const String& name) override;

            virtual void Submit() override;

        private:
            void AllocateNewCommandBuffer();

            bool IsWriting() const { return mIsWriting; }
            bool IsInRenderPass() const { return mRenderEncoder != nil; }

            void EndEncoding();

            MetalTimestampManager& mTimestampManager;

            id<MTLCommandQueue> mCommandQueueRef;
            id<MTLCommandBuffer> mCommandBuffer;
            bool mIsWriting;

            MetalEncoderState mCurrentEncoderState;
            id<MTLRenderCommandEncoder> mRenderEncoder;
            id<MTLComputeCommandEncoder> mComputeEncoder;

            id<MTLBuffer> mIndexBuffer;
            NSUInteger mIndexBufferOffset;
            MTLSize mComputeThreadGroupSize;

            String mDebugName;
        };
    } // namespace gapi
} // namespace cube
