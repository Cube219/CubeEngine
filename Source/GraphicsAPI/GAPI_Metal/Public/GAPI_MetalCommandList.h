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
                Uint64 offset = 0;
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

            void SetViewports(ConstArrayView<Viewport> newViewports);
            void ApplyViewports(id<MTLRenderCommandEncoder> encoder);

            void SetScissors(ConstArrayView<ScissorRect> newScissors);
            void ApplyScissors(id<MTLRenderCommandEncoder> encoder);

            void SetPrimitiveTopology(PrimitiveTopology newPrimitiveTopology);

            void SetConstantBuffer(SharedPtr<BufferSRV> srv, Uint32 index);
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

            virtual void BeginEvent(StringView name) override;
            virtual void EndEvent() override;

            virtual void SetViewports(ConstArrayView<Viewport> viewports) override;
            virtual void SetScissors(ConstArrayView<ScissorRect> scissors) override;
            virtual void SetPrimitiveTopology(PrimitiveTopology primitiveTopology) override;

            virtual void SetGraphicsPipeline(SharedPtr<GraphicsPipeline> graphicsPipeline) override;

            virtual void BeginRenderPass(ArrayView<const ColorAttachment> colors, DepthStencilAttachment depthStencil) override;
            virtual void EndRenderPass() override;

            virtual void BindIndexBuffer(SharedPtr<Buffer> buffer, Uint32 offset) override;

            virtual void Draw(Uint32 numVertices, Uint32 baseVertex, Uint32 numInstances, Uint32 baseInstance) override;
            virtual void DrawIndexed(Uint32 numIndices, Uint32 baseIndex, Uint32 baseVertex, Uint32 numInstances, Uint32 baseInstance) override;

            virtual void SetConstantBuffer(Uint32 index, SharedPtr<BufferSRV> constantBuffer) override;
            virtual void UseResource(SharedPtr<BufferSRV> srv) override;
            virtual void UseResource(SharedPtr<BufferUAV> uav) override;
            virtual void UseResource(SharedPtr<TextureSRV> srv) override;
            virtual void UseResource(SharedPtr<TextureUAV> uav) override;

            virtual void ResourceTransition(TransitionState state) override;
            virtual void ResourceTransition(ArrayView<const TransitionState> states) override;

            virtual void SetComputePipeline(SharedPtr<ComputePipeline> computePipeline) override;
            virtual void DispatchThreads(Uint32 numThreadsX, Uint32 numThreadsY, Uint32 numThreadsZ) override;

            virtual void CopyTexture(SharedPtr<Texture> srcTexture, SharedPtr<Texture> dstTexture) override;

            virtual void BeginTimestamp(StringView name) override;
            virtual void EndTimestamp() override;

            virtual void Submit(bool waitUntilFinished) override;

        private:
            void UseResourceInternal(id<MTLResource> resource, MTLResourceUsage usage);

            void AllocateNewCommandBuffer();

            bool IsWriting() const { return mIsWriting; }
            bool IsInRenderPass() const { return mRenderEncoder != nil; }

            void UseRenderEncoder(MTLRenderPassDescriptor* desc);
            void UseComputeEncoder();
            void UseBlitEncoder();
            void ConsumeTimestampIndexBeforeUseEncoder(NSUInteger& outBeginIndex, NSUInteger& outEndIndex);

            void EndRenderEncoder();
            void EndComputeEncoder();
            void EndBlitEncoder();
            void EndAllEncoders();

            bool HasTimestamps() const { return !mTimestampStack.empty(); }
            bool HasPreBeginTimestamps() const { return !mTimestampStack.empty() && mTimestampStack.back().beginSampleIndex == MetalInvalidSampleIndex; }

            MetalTimestampManager& mTimestampManager;
            struct TimestampBegin
            {
                String name;
                Uint32 beginSampleIndex;
            };
            Vector<TimestampBegin> mTimestampStack;
            Uint32 mLastTimestampSampleIndex;

            id<MTLCommandQueue> mCommandQueueRef;
            id<MTLCommandBuffer> mCommandBuffer;
            bool mIsWriting;

            MetalEncoderState mCurrentEncoderState;
            id<MTLRenderCommandEncoder> mRenderEncoder;
            id<MTLComputeCommandEncoder> mComputeEncoder;
            id<MTLBlitCommandEncoder> mBlitEncoder;

            id<MTLBuffer> mIndexBuffer;
            NSUInteger mIndexBufferOffset;
            MTLSize mComputeThreadGroupSize;

            String mDebugName;
        };
    } // namespace gapi
} // namespace cube
