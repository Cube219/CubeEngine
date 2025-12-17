#pragma once

#include "GAPIHeader.h"

#include "Vector.h"

#include "GAPI_Resource.h"

namespace cube
{
    namespace gapi
    {
        class Buffer;
        class ComputePipeline;
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

        struct TransitionState
        {
            enum class ResourceType
            {
                Buffer,
                SRV,
                UAV,
                RTV,
                DSV
            };
            ResourceType resourceType;

            SharedPtr<Buffer> buffer = nullptr;
            SharedPtr<TextureSRV> srv = nullptr;
            SharedPtr<TextureUAV> uav = nullptr;
            SharedPtr<TextureRTV> rtv = nullptr;
            SharedPtr<TextureDSV> dsv = nullptr;

            ResourceStateFlags src;
            ResourceStateFlags dst;
        };

        struct CommandListCreateInfo
        {
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

            virtual void SetViewports(ArrayView<Viewport> viewports) = 0;
            virtual void SetScissors(ArrayView<ScissorRect> scissors) = 0;
            virtual void SetPrimitiveTopology(PrimitiveTopology primitiveTopology) = 0;

            virtual void SetGraphicsPipeline(SharedPtr<GraphicsPipeline> graphicsPipeline) = 0;
            virtual void SetRenderTargets(ArrayView<ColorAttachment> colors, DepthStencilAttachment depthStencil) = 0;

            virtual void BindVertexBuffers(Uint32 startIndex, ArrayView<SharedPtr<Buffer>> buffers, ArrayView<Uint32> offsets) = 0;
            virtual void BindIndexBuffer(SharedPtr<Buffer> buffer, Uint32 offset) = 0;

            virtual void Draw(Uint32 numVertices, Uint32 baseVertex, Uint32 numInstances = 1, Uint32 baseInstance = 0) = 0;
            virtual void DrawIndexed(Uint32 numIndices, Uint32 baseIndex, Uint32 baseVertex, Uint32 numInstances = 1, Uint32 baseInstance = 0) = 0;

            virtual void SetShaderVariableConstantBuffer(Uint32 index, SharedPtr<Buffer> constantBuffer) = 0;
            virtual void BindTexture(SharedPtr<Texture> texture) = 0;
            virtual void BindSampler(SharedPtr<Sampler> sampler) = 0;

            virtual void ResourceTransition(TransitionState state) = 0;
            virtual void ResourceTransition(ArrayView<TransitionState> states) = 0;

            virtual void SetComputePipeline(SharedPtr<ComputePipeline> computePipeline) = 0;
            virtual void Dispatch(Uint32 threadGroupX, Uint32 threadGroupY, Uint32 threadGroupZ) = 0;

            virtual void InsertTimestamp(const String& name) = 0;

            virtual void Submit() = 0;
        };
    } // namespace gapi
} // namespace cube
