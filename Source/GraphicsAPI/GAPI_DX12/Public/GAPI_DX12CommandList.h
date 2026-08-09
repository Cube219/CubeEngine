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
        class DX12Fence;
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
            virtual void UnsetConstantBuffer(Uint32 index) override;

            virtual void UseResource(SharedPtr<BufferSRV> srv) override;
            virtual void UseResource(SharedPtr<BufferUAV> uav) override;
            virtual void UseResource(SharedPtr<TextureSRV> srv) override;
            virtual void UseResource(SharedPtr<TextureUAV> uav) override;

            virtual void SetResourceBarrier(ResourceBarrier barrier) override;
            virtual void SetResourceBarrier(ConstArrayView<ResourceBarrier> barriers) override;

            void SetComputePipeline(SharedPtr<ComputePipeline> computePipeline) override;
            virtual void DispatchThreads(Uint32 numThreadsX, Uint32 numThreadsY, Uint32 numThreadsZ) override;

            virtual void CopyTexture(SharedPtr<Texture> srcTexture, SharedPtr<Texture> dstTexture) override;
            virtual void CopyBuffer(SharedPtr<Buffer> srcBuffer, Uint64 srcOffset, SharedPtr<Buffer> dstBuffer, Uint64 dstOffset, Uint64 size) override;
            virtual void CopyBufferToTexture(SharedPtr<Buffer> srcBuffer, Uint64 srcOffset, SharedPtr<Texture> dstTexture) override;

            virtual void OptimizeTextureContentsForGPUAccess(SharedPtr<Texture> texture) override;

            virtual void BeginTimestamp(StringView name) override;
            virtual void EndTimestamp() override;

            virtual void WaitForFence(SharedPtr<Fence> fence, Uint64 fenceValue) override;
            virtual void SignalToFence(SharedPtr<Fence> fence, Uint64 fenceValue) override;

            virtual void Submit() override;

            bool IsWriting() const { return mPhase == Phase::Writing; }
            bool IsInRenderPass() const { return mIsInRenderPass; }

        private:
            void ProcessBeforeEnd();
            void InitCommandList(bool createCommandList);
            void ApplyCommandListState();

            ID3D12CommandAllocator* GetCurrentAllocator() const;

            enum class Phase
            {
                Initial,
                Writing,
                Closed
            };

            struct State
            {
                Vector<D3D12_VIEWPORT> viewports;
                Vector<D3D12_RECT> scissors;
                D3D12_PRIMITIVE_TOPOLOGY primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;

                struct ConstantBuffer
                {
                    SharedPtr<BufferSRV> buffer;
                    bool isSet = false;
                };
                Map<Uint32, ConstantBuffer> constantBuffers;

                void Clear()
                {
                    viewports.clear();
                    scissors.clear();
                    primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
                    constantBuffers.clear();
                }
            };

            struct SubmitAction
            {
                enum class Type
                {
                    Execute,
                    Wait,
                    Signal,
                };

                Type type;
                ComPtr<ID3D12GraphicsCommandList7> commandList;
                gapi::DX12Fence* fence;
                Uint64 fenceValue = 0;
            };

            DX12Device& mDevice;

            CommandListType mType;
            // Copy-type command lists own their allocator to reset it independently
            // from the per-frame allocators in the command list manager.
            ComPtr<ID3D12CommandAllocator> mCopyAllocator;

            Phase mPhase;
            ComPtr<ID3D12GraphicsCommandList7> mCommandList;
            State mCommandListState;
            Vector<SubmitAction> mSubmitActions;

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
