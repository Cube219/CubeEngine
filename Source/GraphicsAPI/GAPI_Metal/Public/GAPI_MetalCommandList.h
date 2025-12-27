#pragma once

#include "MetalHeader.h"

#include "GAPI_CommandList.h"

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

            MTLTriangleFillMode fillMode;
            MTLCullMode cullMode;
            MTLWinding winding;
            id<MTLRenderPipelineState> renderPipelineState;
            id<MTLDepthStencilState> depthStencilState;

            Vector<id<MTLBuffer>> vertexBuffers;
            Vector<NSUInteger> vertexBufferOffsets;
            id<MTLBuffer> indexBuffer;
            NSUInteger indexBufferOffset;

            id<MTLComputePipelineState> computePipelineState;
            MTLSize computeThreadGroupSize;

            struct ConstantBuffer
            {
                id<MTLBuffer> buffer;
                bool isSet = false;
            };
            Map<int, ConstantBuffer> constantBuffers;
            struct UsedResource
            {
                id<MTLResource> resource;
                MTLResourceUsage  usage;
            };
            Vector<UsedResource> usedResources;

            void Clear()
            {
                viewports.clear();
                scissors.clear();
                primitiveType = MTLPrimitiveTypeTriangle;
                fillMode = MTLTriangleFillModeFill;
                cullMode = MTLCullModeBack;
                winding = MTLWindingCounterClockwise;
                renderPipelineState = nil;
                depthStencilState = nil;
                vertexBuffers.clear();
                vertexBufferOffsets.clear();
                indexBuffer = nil;
                indexBufferOffset = 0;
                computePipelineState = nil;
                computeThreadGroupSize = MTLSizeMake(0, 0, 0);
                constantBuffers.clear();
                usedResources.clear();
            }

            void SetViewports(ArrayView<Viewport> newViewports);
            void ApplyViewports(id<MTLRenderCommandEncoder> encoder);

            void SetScissors(ArrayView<ScissorRect> newScissors);
            void ApplyScissors(id<MTLRenderCommandEncoder> encoder);

            void SetPrimitiveTopology(PrimitiveTopology newPrimitiveTopology);

            void SetGraphicsPipeline(SharedPtr<GraphicsPipeline> newGraphicsPipeline);
            void ApplyGraphicsPipeline(id<MTLRenderCommandEncoder> encoder);

            void SetVertexBuffers(ArrayView<SharedPtr<Buffer>> buffers, ArrayView<Uint32> offsets);
            void ApplyVertexBuffers(id<MTLRenderCommandEncoder> encoder);

            void SetIndexBuffer(SharedPtr<Buffer> buffer, Uint32 offset);

            void SetComputePipeline(SharedPtr<ComputePipeline> newComputePipeline);
            void ApplyComputePipeline(id<MTLComputeCommandEncoder> encoder);

            void SetConstantBuffer(SharedPtr<Buffer> buffer, Uint32 index);
            void ApplyConstantBuffers(id<MTLRenderCommandEncoder> encoder, bool forceAll = false);
            void ApplyConstantBuffers(id<MTLComputeCommandEncoder> computeEncoder, bool forceAll = false);

            void AddUsedResource(id<MTLResource> resource, MTLResourceUsage usage);
            void ApplyAllUsedResource(id<MTLRenderCommandEncoder> encoder);
            void ApplyAllUsedResource(id<MTLComputeCommandEncoder> computeEncoder);

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
            virtual void SetRenderTargets(ArrayView<ColorAttachment> colors, DepthStencilAttachment depthStencil) override;

            virtual void BindVertexBuffers(Uint32 startIndex, ArrayView<SharedPtr<Buffer>> buffers, ArrayView<Uint32> offsets) override;
            virtual void BindIndexBuffer(SharedPtr<Buffer> buffer, Uint32 offset) override;

            virtual void Draw(Uint32 numVertices, Uint32 baseVertex, Uint32 numInstances, Uint32 baseInstance) override;
            virtual void DrawIndexed(Uint32 numIndices, Uint32 baseIndex, Uint32 baseVertex, Uint32 numInstances, Uint32 baseInstance) override;

            virtual void SetShaderVariableConstantBuffer(Uint32 index, SharedPtr<Buffer> constantBuffer) override;
            virtual void UseResource(SharedPtr<TextureSRV> srv) override;
            virtual void UseResource(SharedPtr<TextureUAV> uav) override;

            virtual void ResourceTransition(TransitionState state) override;
            virtual void ResourceTransition(ArrayView<TransitionState> states) override;

            virtual void SetComputePipeline(SharedPtr<ComputePipeline> computePipeline) override;
            virtual void DispatchThreads(Uint32 numThreadsX, Uint32 numThreadsY, Uint32 numThreadsZ) override;

            virtual void InsertTimestamp(const String& name) override;

            virtual void Submit() override;

        private:
            void AllocateNewCommandBuffer();

            enum class EncoderType
            {
                Graphics,
                Compute
            };

            bool IsWriting() const { return mIsWriting; }
            bool IsWritingInRenderEncoder() const { return mRenderEncoder != nil; }

            void BeginEncoding(EncoderType type);
            void EndEncoding();
            void UpdateEncoderIfNeeded(EncoderType type);

            id<MTLCommandQueue> mCommandQueueRef;
            id<MTLCommandBuffer> mCommandBuffer;
            bool mIsWriting;

            bool mIsNeededRenderEncoderUpdating;

            MetalEncoderState mCurrentEncoderState;
            MTLRenderPassDescriptor* mRenderPassDescriptor;
            id<MTLRenderCommandEncoder> mRenderEncoder;
            id<MTLComputeCommandEncoder> mComputeEncoder;

            String mDebugName;
        };
    } // namespace gapi
} // namespace cube
