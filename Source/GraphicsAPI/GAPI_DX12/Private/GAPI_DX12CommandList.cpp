#include "GAPI_DX12CommandList.h"

#include "Allocator/FrameAllocator.h"
#include "DX12Device.h"
#include "GAPI_DX12Buffer.h"
#include "GAPI_DX12Pipeline.h"
#include "GAPI_DX12Resource.h"
#include "GAPI_DX12ShaderVariable.h"
#include "GAPI_DX12Texture.h"
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

        DX12CommandList::DX12CommandList(DX12Device& device, const CommandListCreateInfo& info)
            : mCommandListManager(device.GetCommandListManager())
            , mDescriptorManager(device.GetDescriptorManager())
            , mQueueManager(device.GetQueueManager())
            , mQueryManager(device.GetQueryManager())
            , mState(State::Closed)
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

        void DX12CommandList::SetViewports(ArrayView<Viewport> viewports)
        {
            CHECK(mState == State::Writing);

            FrameVector<D3D12_VIEWPORT> d3d12Viewports(viewports.size());
            for (int i = 0; i < viewports.size(); ++i)
            {
                const Viewport& vp = viewports[i];
                d3d12Viewports[i] = {
                    .TopLeftX = vp.x,
                    .TopLeftY = vp.y,
                    .Width = vp.width,
                    .Height = vp.height,
                    .MinDepth = vp.minDepth,
                    .MaxDepth = vp.maxDepth
                };
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

        void DX12CommandList::SetGraphicsPipeline(SharedPtr<GraphicsPipeline> graphicsPipeline)
        {
            CHECK(mState == State::Writing);

            mCommandList->SetPipelineState(dynamic_cast<DX12GraphicsPipeline*>(graphicsPipeline.get())->GetPipelineState());
        }

        void DX12CommandList::SetRenderTargets(ArrayView<SharedPtr<TextureRTV>> rtvs, SharedPtr<TextureDSV> dsv)
        {
            CHECK(mState == State::Writing);

            FrameVector<D3D12_CPU_DESCRIPTOR_HANDLE> rtvHandles(rtvs.size());
            for (int i = 0; i < rtvs.size(); ++i)
            {
                DX12TextureRTV* dx12RTV = dynamic_cast<DX12TextureRTV*>(rtvs[i].get());
                CHECK(dx12RTV);
                rtvHandles[i] = dx12RTV->GetDescriptorHandle();
            }

            if (dsv)
            {
                DX12TextureDSV* dx12DSV = dynamic_cast<DX12TextureDSV*>(dsv.get());
                CHECK(dx12DSV);
                D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dx12DSV->GetDescriptorHandle();

                mCommandList->OMSetRenderTargets((UINT)rtvHandles.size(), rtvHandles.data(), false, &dsvHandle);
            }
            else
            {
                mCommandList->OMSetRenderTargets((UINT)rtvHandles.size(), rtvHandles.data(), false, nullptr);
            }
        }

        void DX12CommandList::ClearRenderTargetView(SharedPtr<TextureRTV> rtv, Float4 color)
        {
            CHECK(mState == State::Writing);

            float fColor[4] = { color.x, color.y, color.z, color.w };
            DX12TextureRTV* dx12RTV = dynamic_cast<DX12TextureRTV*>(rtv.get());
            CHECK(dx12RTV);

            mCommandList->ClearRenderTargetView(dx12RTV->GetDescriptorHandle(), fColor, 0, nullptr);
        }

        void DX12CommandList::ClearDepthStencilView(SharedPtr<TextureDSV> dsv, float depth)
        {
            CHECK(mState == State::Writing);

            DX12TextureDSV* dx12DSV = dynamic_cast<DX12TextureDSV*>(dsv.get());
            CHECK(dx12DSV);
            mCommandList->ClearDepthStencilView(dx12DSV->GetDescriptorHandle(), D3D12_CLEAR_FLAG_DEPTH, depth, 0, 0, nullptr);
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
            mCommandList->SetComputeRootSignature(dx12ShaderVariablesLayout->GetRootSignature());

            mIsShaderVariableLayoutSet = true;
        }

        void DX12CommandList::SetShaderVariableConstantBuffer(Uint32 index, SharedPtr<Buffer> constantBuffer)
        {
            CHECK(mState == State::Writing);
            CHECK(mIsShaderVariableLayoutSet);
            CHECK(constantBuffer->GetType() == BufferType::Constant);

            const DX12Buffer* dx12Buffer = dynamic_cast<DX12Buffer*>(constantBuffer.get());

            mCommandList->SetGraphicsRootConstantBufferView(index, dx12Buffer->GetResource()->GetGPUVirtualAddress());
            mCommandList->SetComputeRootConstantBufferView(index, dx12Buffer->GetResource()->GetGPUVirtualAddress());
        }

        void DX12CommandList::BindTexture(SharedPtr<Texture> texture)
        {
            CHECK(mState == State::Writing);

            // Just bind the object
        }

        void DX12CommandList::BindSampler(SharedPtr<Sampler> sampler)
        {
            CHECK(mState == State::Writing);

            // Just bind the object
        }

        void DX12CommandList::ResourceTransition(TransitionState state)
        {
            ResourceTransition({ &state, 1 });
        }

        void DX12CommandList::ResourceTransition(ArrayView<TransitionState> states)
        {
            CHECK(mState == State::Writing);

            FrameVector<D3D12_RESOURCE_BARRIER> barriers;
            barriers.reserve(states.size());

            auto AddTextureSubresourceBarriers = [&barriers, this](const DX12Texture* texture, SubresourceRange range, D3D12_RESOURCE_BARRIER barrier)
            {
                Uint32 arraySize = texture->GetArraySize();
                Uint32 mipLevels = texture->GetMipLevels();
                for (Uint32 arrayIndex = range.firstArrayIndex; arrayIndex < range.firstArrayIndex + range.arraySize; ++arrayIndex)
                {
                    for (Uint32 mipIndex = range.firstMipLevel; mipIndex < range.firstMipLevel + range.mipLevels; ++mipIndex)
                    {
                        barrier.Transition.Subresource = D3D12CalcSubresource(mipIndex, arrayIndex, 0, mipLevels, arraySize);
                        barriers.push_back(barrier);
                    }
                }
            };

            for (const TransitionState state : states)
            {
                D3D12_RESOURCE_BARRIER barrier = {
                    .Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
                    .Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
                    .Transition = {
                        .StateBefore = ConvertToDX12ResourceStates(state.src),
                        .StateAfter = ConvertToDX12ResourceStates(state.dst) }
                };

                switch (state.resourceType)
                {
                case TransitionState::ResourceType::Buffer:
                {
                    const DX12Buffer* dx12Buffer = dynamic_cast<DX12Buffer*>(state.buffer.get());
                    barrier.Transition.pResource = dx12Buffer->GetResource();
                    barrier.Transition.Subresource = 0;
                    barriers.push_back(barrier);
                    break;
                }
                case TransitionState::ResourceType::SRV:
                {
                    const DX12TextureSRV* dx12SRV = dynamic_cast<DX12TextureSRV*>(state.srv.get());
                    const DX12Texture* dx12Texture = dx12SRV->GetDX12Texture();
                    barrier.Transition.pResource = dx12Texture->GetResource();
                    AddTextureSubresourceBarriers(dx12Texture, dx12SRV->GetSubresourceRange(), barrier);
                    break;
                }
                case TransitionState::ResourceType::UAV:
                {
                    const DX12TextureUAV* dx12UAV = dynamic_cast<DX12TextureUAV*>(state.uav.get());
                    const DX12Texture* dx12Texture = dx12UAV->GetDX12Texture();
                    barrier.Transition.pResource = dx12UAV->GetDX12Texture()->GetResource();
                    AddTextureSubresourceBarriers(dx12Texture, dx12UAV->GetSubresourceRange(), barrier);
                    break;
                }
                case TransitionState::ResourceType::RTV:
                {
                    const DX12TextureRTV* dx12RTV = dynamic_cast<DX12TextureRTV*>(state.rtv.get());
                    const DX12Texture* dx12Texture = dx12RTV->GetDX12Texture();
                    barrier.Transition.pResource = dx12RTV->GetDX12Texture()->GetResource();
                    AddTextureSubresourceBarriers(dx12Texture, dx12RTV->GetSubresourceRange(), barrier);
                    break;
                }
                case TransitionState::ResourceType::DSV:
                {
                    const DX12TextureDSV* dx12DSV = dynamic_cast<DX12TextureDSV*>(state.dsv.get());
                    const DX12Texture* dx12Texture = dx12DSV->GetDX12Texture();
                    barrier.Transition.pResource = dx12DSV->GetDX12Texture()->GetResource();
                    AddTextureSubresourceBarriers(dx12Texture, dx12DSV->GetSubresourceRange(), barrier);
                    break;
                }
                default:
                    NOT_IMPLEMENTED();
                    break;
                }
            }

            mCommandList->ResourceBarrier(barriers.size(), barriers.data());
        }

        void DX12CommandList::SetComputePipeline(SharedPtr<ComputePipeline> computePipeline)
        {
            CHECK(mState == State::Writing);

            mCommandList->SetPipelineState(dynamic_cast<DX12ComputePipeline*>(computePipeline.get())->GetPipelineState());
        }

        void DX12CommandList::Dispatch(Uint32 threadGroupX, Uint32 threadGroupY, Uint32 threadGroupZ)
        {
            CHECK(mState == State::Writing);

            mCommandList->Dispatch(threadGroupX, threadGroupY, threadGroupZ);
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
        }
    } // namespace gapi
} // namespace cube
