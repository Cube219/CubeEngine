#include "GAPI_DX12CommandList.h"

#include "Allocator/FrameAllocator.h"
#include "DX12Device.h"
#include "GAPI_DX12Buffer.h"
#include "GAPI_DX12Pipeline.h"
#include "GAPI_DX12Resource.h"
#include "GAPI_DX12ShaderVariable.h"
#include "GAPI_DX12Viewport.h"
#include "GAPI_Sampler.h"
#include "GAPI_Texture.h"
#include "Renderer/RenderTypes.h"

namespace cube
{
    namespace gapi
    {
        D3D12_PRIMITIVE_TOPOLOGY ConvertToDX12PrimitiveTopology(PrimitiveTopology primitiveTopology)
        {
            switch (primitiveTopology)
            {
            case PrimitiveTopology::PointList:
                return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
            case PrimitiveTopology::LineList:
                return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
            case PrimitiveTopology::LineStrip:
                return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
            case PrimitiveTopology::TriangleList:
                return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            case PrimitiveTopology::TriangleStrip:
                return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
            default:
                NOT_IMPLEMENTED();
            }
            return (D3D12_PRIMITIVE_TOPOLOGY)0;
        }

        DX12CommandList::DX12CommandList(DX12Device& device, const CommandListCreateInfo& info) :
            mCommandListManager(device.GetCommandListManager()),
            mDescriptorManager(device.GetDescriptorManager()),
            mQueueManager(device.GetQueueManager()),
            mQueryManager(device.GetQueryManager()),
            mState(State::Closed)
        {
            device.GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mCommandListManager.GetCurrentAllocator(), nullptr, IID_PPV_ARGS(&mCommandList));
            SET_DEBUG_NAME(mCommandList, info.debugName);
            CHECK_HR(mCommandList->Close());
        }

        DX12CommandList::~DX12CommandList()
        {
            mCommandList = nullptr;
        }

        void DX12CommandList::Begin()
        {
            CHECK(mState == State::Initial);

            mState = State::Writing;
        }

        void DX12CommandList::End()
        {
            CHECK(mState == State::Writing);

            if (mHasTimestampQuery)
            {
                mQueryManager.ResolveTimestampQueryData(mCommandList.Get());
            }

            CHECK_HR(mCommandList->Close());
            mState = State::Closed;
        }

        void DX12CommandList::Reset()
        {
            if (mState == State::Initial)
            {
                return;
            }

            CHECK(mState == State::Closed);

            CHECK_HR(mCommandList->Reset(mCommandListManager.GetCurrentAllocator(), nullptr));
            mIsDescriptorHeapSet = false;
            mIsShaderVariableLayoutSet = false;
            mHasTimestampQuery = false;
            mState = State::Initial;
        }

        void DX12CommandList::SetViewports(ArrayView<SharedPtr<Viewport>> viewports)
        {
            CHECK(mState == State::Writing);

            FrameVector<D3D12_VIEWPORT> d3d12Viewports(viewports.size());
            for (int i = 0; i < viewports.size(); ++i)
            {
                const DX12Viewport* dxViewport = dynamic_cast<DX12Viewport*>(viewports[i].get());
                d3d12Viewports[i] = dxViewport->GetD3D12Viewport();

                CUBE_DX12_BOUND_OBJECT(viewports[i]);
            }

            mCommandList->RSSetViewports(d3d12Viewports.size(), d3d12Viewports.data());
        }

        void DX12CommandList::SetScissors(ArrayView<ScissorRect> scissors)
        {
            CHECK(mState == State::Writing);

            FrameVector<D3D12_RECT> d3d12Rects(scissors.size());
            for (int i = 0; i < scissors.size(); ++i)
            {
                d3d12Rects[i] = {
                    .left = scissors[i].x,
                    .top = scissors[i].y,
                    .right = static_cast<LONG>(scissors[i].x + scissors[i].width - 1),
                    .bottom = static_cast<LONG>(scissors[i].y + scissors[i].height - 1)
                };
            }
            mCommandList->RSSetScissorRects(d3d12Rects.size(), d3d12Rects.data());
        }

        void DX12CommandList::SetPrimitiveTopology(PrimitiveTopology primitiveTopology)
        {
            CHECK(mState == State::Writing);

            mCommandList->IASetPrimitiveTopology(ConvertToDX12PrimitiveTopology(primitiveTopology));
        }

        void DX12CommandList::SetShaderVariablesLayout(SharedPtr<ShaderVariablesLayout> shaderVariablesLayout)
        {
            CHECK(mState == State::Writing);

            if (!mIsDescriptorHeapSet)
            {
                ArrayView<ID3D12DescriptorHeap*> heaps = mDescriptorManager.GetD3D12ShaderVisibleHeaps();
                mCommandList->SetDescriptorHeaps(heaps.size(), heaps.data());
                mIsDescriptorHeapSet = true;
            }

            const DX12ShaderVariablesLayout* dx12ShaderVariablesLayout = dynamic_cast<const DX12ShaderVariablesLayout*>(shaderVariablesLayout.get());
            mCommandList->SetGraphicsRootSignature(dx12ShaderVariablesLayout->GetRootSignature());

            CUBE_DX12_BOUND_OBJECT(shaderVariablesLayout);

            mIsShaderVariableLayoutSet = true;
        }

        void DX12CommandList::SetGraphicsPipeline(SharedPtr<Pipeline> graphicsPipeline)
        {
            CHECK(mState == State::Writing);

            mCommandList->SetPipelineState(dynamic_cast<DX12Pipeline*>(graphicsPipeline.get())->GetPipelineState());

            CUBE_DX12_BOUND_OBJECT(graphicsPipeline);
        }

        void DX12CommandList::SetRenderTarget(SharedPtr<Viewport> viewport)
        {
            CHECK(mState == State::Writing);

            DX12Viewport* dx12Viewport = dynamic_cast<DX12Viewport*>(viewport.get());
            D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = dx12Viewport->GetCurrentRTVDescriptor();
            D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dx12Viewport->GetDSVDescriptor().handle;
            mCommandList->OMSetRenderTargets(1, &rtvHandle, false, &dsvHandle);

            CUBE_DX12_BOUND_OBJECT(viewport);
        }

        void DX12CommandList::ClearRenderTargetView(SharedPtr<Viewport> viewport, Float4 color)
        {
            CHECK(mState == State::Writing);

            float fColor[4] = { color.x, color.y, color.z, color.w };
            DX12Viewport* dx12Viewport = dynamic_cast<DX12Viewport*>(viewport.get());
            D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = dx12Viewport->GetCurrentRTVDescriptor();
            mCommandList->ClearRenderTargetView(rtvHandle, fColor, 0, nullptr);

            CUBE_DX12_BOUND_OBJECT(viewport);
        }

        void DX12CommandList::ClearDepthStencilView(SharedPtr<Viewport> viewport, float depth)
        {
            CHECK(mState == State::Writing);

            DX12Viewport* dx12Viewport = dynamic_cast<DX12Viewport*>(viewport.get());
            D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dx12Viewport->GetDSVDescriptor().handle;
            mCommandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, depth, 0, 0, nullptr);

            CUBE_DX12_BOUND_OBJECT(viewport);
        }

        void DX12CommandList::SetShaderVariableConstantBuffer(Uint32 index, SharedPtr<Buffer> constantBuffer)
        {
            CHECK(mState == State::Writing);
            CHECK(mIsShaderVariableLayoutSet);
            CHECK(constantBuffer->GetType() == BufferType::Constant);

            const DX12Buffer* dx12Buffer = dynamic_cast<DX12Buffer*>(constantBuffer.get());

            mCommandList->SetGraphicsRootConstantBufferView(index, dx12Buffer->GetResource()->GetGPUVirtualAddress());

            CUBE_DX12_BOUND_OBJECT(constantBuffer);
        }

        void DX12CommandList::BindTexture(SharedPtr<Texture> texture)
        {
            CHECK(mState == State::Writing);

            // Just bind the object
            CUBE_DX12_BOUND_OBJECT(texture);
        }

        void DX12CommandList::BindSampler(SharedPtr<Sampler> sampler)
        {
            CHECK(mState == State::Writing);

            // Just bind the object
            CUBE_DX12_BOUND_OBJECT(sampler);
        }

        void DX12CommandList::ResourceTransition(SharedPtr<Buffer> buffer, ResourceStateFlags srcState, ResourceStateFlags dstState)
        {
            CHECK(mState == State::Writing);

            const DX12Buffer* dx12Buffer = dynamic_cast<DX12Buffer*>(buffer.get());

            D3D12_RESOURCE_BARRIER barrier = {
                .Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
                .Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
                .Transition = {
                    .pResource = dx12Buffer->GetResource(),
                    .Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
                    .StateBefore = ConvertToDX12ResourceStates(srcState),
                    .StateAfter = ConvertToDX12ResourceStates(dstState)
                }
            };
            mCommandList->ResourceBarrier(1, &barrier);

            CUBE_DX12_BOUND_OBJECT(buffer);
        }

        void DX12CommandList::ResourceTransition(SharedPtr<Viewport> viewport, ResourceStateFlags srcState, ResourceStateFlags dstState)
        {
            CHECK(mState == State::Writing);

            ID3D12Resource* currentBackbuffer = dynamic_cast<DX12Viewport*>(viewport.get())->GetCurrentBackbuffer();

            D3D12_RESOURCE_BARRIER barrier = {
                .Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
                .Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
                .Transition = {
                    .pResource = currentBackbuffer,
                    .Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
                    .StateBefore = ConvertToDX12ResourceStates(srcState),
                    .StateAfter = ConvertToDX12ResourceStates(dstState)
                }
            };
            mCommandList->ResourceBarrier(1, &barrier);

            CUBE_DX12_BOUND_OBJECT(viewport);
        }

        void DX12CommandList::BindVertexBuffers(Uint32 startIndex, ArrayView<SharedPtr<Buffer>> buffers, ArrayView<Uint32> offsets)
        {
            CHECK(mState == State::Writing);
            CHECK(buffers.size() == offsets.size());

            FrameVector<D3D12_VERTEX_BUFFER_VIEW> d3d12VertexBufferViews(buffers.size());
            for (Uint32 i = 0; i < buffers.size(); ++i)
            {
                CHECK(buffers[i]->GetType() == BufferType::Vertex);

                const DX12Buffer* dx12Buffer = dynamic_cast<DX12Buffer*>(buffers[i].get());
                D3D12_VERTEX_BUFFER_VIEW& vertexBufferView = d3d12VertexBufferViews[i];

                vertexBufferView = {
                    .BufferLocation = dx12Buffer->GetResource()->GetGPUVirtualAddress(),
                    .SizeInBytes = static_cast<UINT>(dx12Buffer->GetSize()),
                    .StrideInBytes = sizeof(Vertex)
                };

                CUBE_DX12_BOUND_OBJECT(buffers[i]);
            }

            mCommandList->IASetVertexBuffers(0, d3d12VertexBufferViews.size(), d3d12VertexBufferViews.data());
        }

        void DX12CommandList::BindIndexBuffer(SharedPtr<Buffer> buffer, Uint32 offset)
        {
            CHECK(mState == State::Writing);
            CHECK(buffer->GetType() == BufferType::Index);

            const DX12Buffer* dx12Buffer = dynamic_cast<DX12Buffer*>(buffer.get());

            const D3D12_INDEX_BUFFER_VIEW indexBufferView = {
                .BufferLocation = dx12Buffer->GetResource()->GetGPUVirtualAddress(),
                .SizeInBytes = static_cast<UINT>(dx12Buffer->GetSize()),
                .Format = (sizeof(Index) == 16 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT)
            };

            mCommandList->IASetIndexBuffer(&indexBufferView);

            CUBE_DX12_BOUND_OBJECT(buffer);
        }

        void DX12CommandList::Draw(Uint32 numVertices, Uint32 baseVertex, Uint32 numInstances, Uint32 baseInstance)
        {
            CHECK(mState == State::Writing);

            mCommandList->DrawInstanced(numVertices, numInstances, baseVertex, baseInstance);
        }

        void DX12CommandList::DrawIndexed(Uint32 numIndices, Uint32 baseIndex, Uint32 baseVertex, Uint32 numInstances, Uint32 baseInstance)
        {
            CHECK(mState == State::Writing);

            mCommandList->DrawIndexedInstanced(numIndices, numInstances, baseIndex, baseVertex, baseInstance);
        }

        void DX12CommandList::InsertTimestamp(const String& name)
        {
            CHECK(mState == State::Writing);

            int index = mQueryManager.AddTimestamp(name);
            mCommandList->EndQuery(mQueryManager.GetCurrentTimestampHeap(), D3D12_QUERY_TYPE_TIMESTAMP, index);

            mHasTimestampQuery = true;
        }

        void DX12CommandList::Submit()
        {
            CHECK(mState == State::Closed);

            ID3D12CommandList* cmdLists[] = { mCommandList.Get() };
            mQueueManager.GetMainQueue()->ExecuteCommandLists(1, cmdLists);

            mCommandListManager.AddBoundObjects(mBoundObjects);
            mBoundObjects.clear();
        }
    } // namespace gapi
} // namespace cube
