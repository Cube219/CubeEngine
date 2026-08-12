#pragma once

#include "GAPIHeader.h"

#include "Vector.h"

#include "GAPI_Resource.h"

namespace cube
{
    namespace gapi
    {
        class Buffer;
        class BufferSRV;
        class BufferUAV;
        class ComputePipeline;
        class Fence;
        class GraphicsPipeline;
        class Sampler;
        class Texture;
        class TextureDSV;
        class TextureRTV;
        class TextureSRV;
        class TextureUAV;

        struct Viewport
        {
            float x;
            float y;
            float width;
            float height;
            float minDepth = 0.0f;
            float maxDepth = 1.0f;
        };

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

        enum class LoadOperation
        {
            DontCare,
            Load,
            Clear
        };

        enum class StoreOperation
        {
            DontCare,
            Store
        };

        struct ColorAttachment
        {
            SharedPtr<TextureRTV> rtv = nullptr;
            LoadOperation loadOperation = LoadOperation::Load;
            StoreOperation storeOperation = StoreOperation::Store;
            Float4 clearColor;
        };

        struct DepthStencilAttachment
        {
            SharedPtr<TextureDSV> dsv = nullptr;
            LoadOperation loadOperation = LoadOperation::Load;
            StoreOperation storeOperation = StoreOperation::Store;
            float clearDepth;
        };

        struct ResourceBarrier
        {
            enum class ResourceType
            {
                Buffer,
                Texture,
            };
            ResourceType resourceType;

            SharedPtr<Buffer> buffer = nullptr;
            SharedPtr<Texture> texture = nullptr;
            // < 0 : All subresources.
            Int32 subresourceIndex = -1;
            bool discard = false;

            ResourceSyncFlags syncSrc;
            ResourceSyncFlags syncDst;
            ResourceAccessFlags accessSrc;
            ResourceAccessFlags accessDst;
            ResourceLayout layoutSrc;
            ResourceLayout layoutDst;
        };

        enum class CommandListType
        {
            Direct,
            Copy
        };

        struct CommandListCreateInfo
        {
            CommandListType type = CommandListType::Direct;

            StringView debugName;
        };

        class CommandList
        {
        public:
            CommandList() = default;
            virtual ~CommandList() = default;

            virtual void Begin() = 0;
            virtual void End() = 0;
            virtual void Reset() = 0;

            virtual void BeginEvent(StringView name) = 0;
            virtual void EndEvent() = 0;

            virtual void SetViewports(ConstArrayView<Viewport> viewports) = 0;
            virtual void SetScissors(ConstArrayView<ScissorRect> scissors) = 0;
            virtual void SetPrimitiveTopology(PrimitiveTopology primitiveTopology) = 0;

            virtual void SetGraphicsPipeline(SharedPtr<GraphicsPipeline> graphicsPipeline) = 0;

            virtual void BeginRenderPass(ArrayView<const ColorAttachment> colors, DepthStencilAttachment depthStencil) = 0;
            virtual void EndRenderPass() = 0;

            virtual void BindIndexBuffer(SharedPtr<Buffer> buffer, Uint32 offset) = 0;

            virtual void Draw(Uint32 numVertices, Uint32 baseVertex, Uint32 numInstances = 1, Uint32 baseInstance = 0) = 0;
            virtual void DrawIndexed(Uint32 numIndices, Uint32 baseIndex, Uint32 baseVertex, Uint32 numInstances = 1, Uint32 baseInstance = 0) = 0;

            virtual void SetConstantBuffer(Uint32 index, SharedPtr<BufferSRV> constantBuffer) = 0;
            virtual void UnsetConstantBuffer(Uint32 index) = 0;

            virtual void UseResource(SharedPtr<BufferSRV> srv) = 0;
            virtual void UseResource(SharedPtr<BufferUAV> uav) = 0;
            virtual void UseResource(SharedPtr<TextureSRV> srv) = 0;
            virtual void UseResource(SharedPtr<TextureUAV> uav) = 0;

            virtual void SetResourceBarrier(ResourceBarrier barrier) = 0;
            virtual void SetResourceBarrier(ConstArrayView<ResourceBarrier> barriers) = 0;

            virtual void SetComputePipeline(SharedPtr<ComputePipeline> computePipeline) = 0;
            virtual void DispatchThreads(Uint32 numThreadsX, Uint32 numThreadsY, Uint32 numThreadsZ) = 0;

            virtual void CopyTexture(SharedPtr<Texture> srcTexture, SharedPtr<Texture> dstTexture) = 0;
            virtual void CopyBuffer(SharedPtr<Buffer> srcBuffer, Uint64 srcOffset, SharedPtr<Buffer> dstBuffer, Uint64 dstOffset, Uint64 size) = 0;
            // The source data must be laid out according to dstTexture->GetSubresourceLayout(), relative to srcOffset.
            virtual void CopyBufferToTexture(SharedPtr<Buffer> srcBuffer, Uint64 srcOffset, SharedPtr<Texture> dstTexture) = 0;

            // Required on Metal after copying to a GPUOnly texture. No-op on DX12.
            virtual void OptimizeTextureContentsForGPUAccess(SharedPtr<Texture> texture) = 0;

            virtual void BeginTimestamp(StringView name) = 0;
            virtual void EndTimestamp() = 0;

            virtual void WaitForFence(SharedPtr<Fence> fence, Uint64 fenceValue) = 0;
            virtual void SignalToFence(SharedPtr<Fence> fence, Uint64 fenceValue) = 0;

            virtual void Submit() = 0;
        };
    } // namespace gapi
} // namespace cube
