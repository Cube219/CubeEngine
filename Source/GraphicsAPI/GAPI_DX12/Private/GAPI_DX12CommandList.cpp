#include "GAPI_DX12CommandList.h"

#include <WinPixEventRuntime/pix3.h>

#include "Allocator/FrameAllocator.h"
#include "DX12Device.h"
#include "GAPI_DX12Buffer.h"
#include "GAPI_DX12Fence.h"
#include "GAPI_DX12Pipeline.h"
#include "GAPI_DX12Resource.h"
#include "GAPI_DX12Texture.h"
#include "GAPI_Sampler.h"
#include "GAPI_Texture.h"
#include "RenderCore/RenderTypes.h"

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
            : mDevice(device)
            , mType(info.type)
            , mCurrentCopyAllocator(nullptr)
            , mPhase(Phase::Closed)
        {
            D3D12_COMMAND_LIST_TYPE commandListType = D3D12_COMMAND_LIST_TYPE_DIRECT;
            if (mType == CommandListType::Copy)
            {
                commandListType = D3D12_COMMAND_LIST_TYPE_COPY;
                mCurrentCopyAllocator = device.GetCommandListManager().GetCurrentCopyAllocator();
            }

            device.GetDevice()->CreateCommandList(0, commandListType, GetCurrentAllocator(), nullptr, IID_PPV_ARGS(&mCommandList));
            SET_DEBUG_NAME(mCommandList, info.debugName);
            CHECK_HR(mCommandList->Close());
        }

        DX12CommandList::~DX12CommandList()
        {
            mCommandList = nullptr;
        }

        void DX12CommandList::Begin()
        {
            CHECK(mPhase == Phase::Initial);

            mHasQuery = false;
            mTimestampStack.clear();
            mCommandListState.Clear();
            mSubmitActions.clear();

            InitCommandList(false);
        }

        void DX12CommandList::End()
        {
            CHECK(IsWriting());

            ProcessBeforeEnd();

            CHECK_FORMAT(mCurrentEventNameList.empty(), "Not all events are ended.");
            CHECK_FORMAT(mTimestampStack.empty(), "Not all timestamps are ended.");

            if (mHasQuery)
            {
                mDevice.GetQueryManager().ResolveQueryData(mCommandList.Get());
            }

            CHECK_HR(mCommandList->Close());
            mPhase = Phase::Closed;
        }

        void DX12CommandList::Reset()
        {
            if (mPhase == Phase::Initial)
            {
                return;
            }

            CHECK(mPhase == Phase::Closed);

            if (mType == CommandListType::Copy)
            {
                mCurrentCopyAllocator = mDevice.GetCommandListManager().GetCurrentCopyAllocator();
            }

            CHECK_HR(mCommandList->Reset(GetCurrentAllocator(), nullptr));
            mHasQuery = false;
            mCommandListState.Clear();
            mSubmitActions.clear();
            mPhase = Phase::Initial;
        }

        void DX12CommandList::BeginEvent(StringView name)
        {
            CHECK(IsWriting());

            AnsiString nameAnsi = String_Convert<AnsiString>(name);
            mCurrentEventNameList.push_back(std::move(nameAnsi));

            PIXBeginEvent(mCommandList.Get(), PIX_COLOR(0xff, 0xff, 0xff), mCurrentEventNameList.back().c_str());
        }

        void DX12CommandList::EndEvent()
        {
            CHECK(IsWriting());
            CHECK_FORMAT(mCurrentEventNameList.size() > 0, "Try to end event but none of events are set currently.");

            PIXEndEvent(mCommandList.Get());
            mCurrentEventNameList.pop_back();
        }

        void DX12CommandList::SetViewports(ConstArrayView<Viewport> viewports)
        {
            CHECK(IsWriting());

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
            mCommandListState.viewports.assign(d3d12Viewports.begin(), d3d12Viewports.end());
        }

        void DX12CommandList::SetScissors(ConstArrayView<ScissorRect> scissors)
        {
            CHECK(IsWriting());

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
            mCommandListState.scissors.assign(d3d12Rects.begin(), d3d12Rects.end());
        }

        void DX12CommandList::SetPrimitiveTopology(PrimitiveTopology primitiveTopology)
        {
            CHECK(IsWriting());

            mCommandList->IASetPrimitiveTopology(ConvertToDX12PrimitiveTopology(primitiveTopology));
            mCommandListState.primitiveTopology = ConvertToDX12PrimitiveTopology(primitiveTopology);
        }

        void DX12CommandList::SetGraphicsPipeline(SharedPtr<GraphicsPipeline> graphicsPipeline)
        {
            CHECK(IsWriting());
            CHECK(IsInRenderPass());

            mCommandListState.computePipelineState = nullptr;
            mCommandList->SetPipelineState(dynamic_cast<DX12GraphicsPipeline*>(graphicsPipeline.get())->GetPipelineState());

            CUBE_DX12_BOUND_OBJECT(graphicsPipeline);
        }

        void DX12CommandList::BeginRenderPass(ArrayView<const ColorAttachment> colors, DepthStencilAttachment depthStencil)
        {
            CHECK(IsWriting());
            CHECK(!IsInRenderPass());
            CHECK(mType == CommandListType::Direct);

            FrameVector<D3D12_CPU_DESCRIPTOR_HANDLE> rtvHandles(colors.size());
            for (int i = 0; i < colors.size(); ++i)
            {
                DX12TextureRTV* dx12RTV = dynamic_cast<DX12TextureRTV*>(colors[i].rtv.get());
                CHECK(dx12RTV);
                rtvHandles[i] = dx12RTV->GetDescriptorHandle();

                CUBE_DX12_BOUND_OBJECT(colors[i].rtv);
            }

            if (depthStencil.dsv)
            {
                DX12TextureDSV* dx12DSV = dynamic_cast<DX12TextureDSV*>(depthStencil.dsv.get());
                CHECK(dx12DSV);
                D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dx12DSV->GetDescriptorHandle();

                CUBE_DX12_BOUND_OBJECT(depthStencil.dsv);

                mCommandList->OMSetRenderTargets((UINT)rtvHandles.size(), rtvHandles.data(), false, &dsvHandle);
            }
            else
            {
                mCommandList->OMSetRenderTargets((UINT)rtvHandles.size(), rtvHandles.data(), false, nullptr);
            }

            // Clear RTVs / DSV if needed
            for (int i = 0; i < colors.size(); ++i)
            {
                if (colors[i].loadOperation == LoadOperation::Clear)
                {
                    DX12TextureRTV* dx12RTV = dynamic_cast<DX12TextureRTV*>(colors[i].rtv.get());
                    CHECK(dx12RTV);
                    float fColor[4] = { colors[i].clearColor.x, colors[i].clearColor.y, colors[i].clearColor.z, colors[i].clearColor.w };
                    mCommandList->ClearRenderTargetView(dx12RTV->GetDescriptorHandle(), fColor, 0, nullptr);
                }
            }
            if (depthStencil.dsv && depthStencil.loadOperation == LoadOperation::Clear)
            {
                DX12TextureDSV* dx12DSV = dynamic_cast<DX12TextureDSV*>(depthStencil.dsv.get());
                CHECK(dx12DSV);
                mCommandList->ClearDepthStencilView(dx12DSV->GetDescriptorHandle(), D3D12_CLEAR_FLAG_DEPTH, depthStencil.clearDepth, 0, 0, nullptr);
            }

            mIsInRenderPass = true;
        }

        void DX12CommandList::EndRenderPass()
        {
            CHECK(IsWriting());
            CHECK(IsInRenderPass());

            mIsInRenderPass = false;
            // DX12 does not have a render pass concept. Do nothing.
        }

        void DX12CommandList::BindIndexBuffer(SharedPtr<Buffer> buffer, Uint32 offset)
        {
            CHECK(IsWriting());
            CHECK(IsInRenderPass());

            const DX12Buffer* dx12Buffer = dynamic_cast<DX12Buffer*>(buffer.get());
            const Uint32 stride = dx12Buffer->GetStride();
            CHECK_FORMAT(stride == 4 || stride == 2, "Index buffer's stride must be 2(16bits) or 4(32bits).");

            const D3D12_INDEX_BUFFER_VIEW indexBufferView = {
                .BufferLocation = dx12Buffer->GetResource()->GetGPUVirtualAddress(),
                .SizeInBytes = static_cast<UINT>(dx12Buffer->GetSize()),
                .Format = (stride == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT)
            };

            mCommandList->IASetIndexBuffer(&indexBufferView);

            CUBE_DX12_BOUND_OBJECT(buffer);
        }

        void DX12CommandList::Draw(Uint32 numVertices, Uint32 baseVertex, Uint32 numInstances, Uint32 baseInstance)
        {
            CHECK(IsWriting());
            CHECK(IsInRenderPass());
            CHECK(mType == CommandListType::Direct);

            mCommandList->DrawInstanced(numVertices, numInstances, baseVertex, baseInstance);
        }

        void DX12CommandList::DrawIndexed(Uint32 numIndices, Uint32 baseIndex, Uint32 baseVertex, Uint32 numInstances, Uint32 baseInstance)
        {
            CHECK(IsWriting());
            CHECK(IsInRenderPass());
            CHECK(mType == CommandListType::Direct);

            mCommandList->DrawIndexedInstanced(numIndices, numInstances, baseIndex, baseVertex, baseInstance);
        }

        void DX12CommandList::SetConstantBuffer(Uint32 index, SharedPtr<BufferSRV> constantBuffer)
        {
            CHECK(IsWriting());
            CHECK(constantBuffer->GetBuffer()->GetType() == BufferType::Constant);

            const DX12BufferSRV* dx12SRV = dynamic_cast<DX12BufferSRV*>(constantBuffer.get());
            CHECK(dx12SRV);

            D3D12_GPU_VIRTUAL_ADDRESS gpuAddress = dx12SRV->GetGPUAddress();

            // Register space index is used in Slang's ParameterBlock.
            DX12ShaderParameterHelper& shaderParameterHelper = mDevice.GetShaderParameterHelper();
            CHECK(index < shaderParameterHelper.GetMaxNumSpace());
            mCommandList->SetGraphicsRootConstantBufferView(index * shaderParameterHelper.GetMaxNumRegister(), gpuAddress);
            mCommandList->SetComputeRootConstantBufferView(index * shaderParameterHelper.GetMaxNumRegister(), gpuAddress);

            mCommandListState.constantBuffers[index] = {
                .buffer = constantBuffer,
                .isSet = true
            };

            CUBE_DX12_BOUND_OBJECT(constantBuffer);
        }

        void DX12CommandList::UnsetConstantBuffer(Uint32 index)
        {
            CHECK(IsWriting());

            mCommandListState.constantBuffers.erase(index);

            // Do nothing. Maybe setting a null GPU address in the root constant buffer view provide no performance benefit.
        }

        void DX12CommandList::UseResource(SharedPtr<BufferSRV> srv)
        {
            CHECK(IsWriting());

            // Just bind the object
            CUBE_DX12_BOUND_OBJECT(srv);
        }

        void DX12CommandList::UseResource(SharedPtr<BufferUAV> uav)
        {
            CHECK(IsWriting());

            // Just bind the object
            CUBE_DX12_BOUND_OBJECT(uav);
        }

        void DX12CommandList::UseResource(SharedPtr<TextureSRV> srv)
        {
            CHECK(IsWriting());

            // Just bind the object
            CUBE_DX12_BOUND_OBJECT(srv);
        }

        void DX12CommandList::UseResource(SharedPtr<TextureUAV> uav)
        {
            CHECK(IsWriting());

            // Just bind the object
            CUBE_DX12_BOUND_OBJECT(uav);
        }

        void DX12CommandList::SetResourceBarrier(ResourceBarrier barrier)
        {
            SetResourceBarrier({ &barrier, 1 });
        }

        void DX12CommandList::SetResourceBarrier(ConstArrayView<ResourceBarrier> barriers)
        {
            CHECK(IsWriting());

            FrameVector<D3D12_BUFFER_BARRIER> bufferBarriers;
            bufferBarriers.reserve(barriers.size());
            FrameVector<D3D12_TEXTURE_BARRIER> textureBarriers;
            textureBarriers.reserve(barriers.size());

            for (const ResourceBarrier& barrier : barriers)
            {
                switch (barrier.resourceType)
                {
                case ResourceBarrier::ResourceType::Buffer:
                {
                    const DX12Buffer* dx12Buffer = dynamic_cast<DX12Buffer*>(barrier.buffer.get());
                    CHECK(dx12Buffer);

                    bufferBarriers.push_back({
                        .SyncBefore = ConvertToDX12ResourceSyncFlags(barrier.syncSrc),
                        .SyncAfter = ConvertToDX12ResourceSyncFlags(barrier.syncDst),
                        .AccessBefore = ConvertToDX12ResourceAccessFlags(barrier.accessSrc),
                        .AccessAfter = ConvertToDX12ResourceAccessFlags(barrier.accessDst),
                        .pResource = dx12Buffer->GetResource(),
                        .Offset = 0,
                        .Size = UINT64_MAX,
                    });

                    CUBE_DX12_BOUND_OBJECT(barrier.buffer);
                    break;
                }
                case ResourceBarrier::ResourceType::Texture:
                {
                    const DX12Texture* dx12Texture = dynamic_cast<DX12Texture*>(barrier.texture.get());
                    CHECK(dx12Texture);

                    CHECK_FORMAT(!barrier.discard || barrier.layoutSrc == ResourceLayout::Undefined, "Source layout must be undefined if the discard option is enabled.");

                    const D3D12_BARRIER_SUBRESOURCE_RANGE subresourceRange = {
                        .IndexOrFirstMipLevel = barrier.subresourceIndex >= 0 ? barrier.subresourceIndex : 0xffffffff,
                        .NumMipLevels = 0,
                        .FirstArraySlice = 0,
                        .NumArraySlices = 0,
                        .FirstPlane = 0,
                        .NumPlanes = 0,
                    };

                    textureBarriers.push_back({
                        .SyncBefore = ConvertToDX12ResourceSyncFlags(barrier.syncSrc),
                        .SyncAfter = ConvertToDX12ResourceSyncFlags(barrier.syncDst),
                        .AccessBefore = ConvertToDX12ResourceAccessFlags(barrier.accessSrc),
                        .AccessAfter = ConvertToDX12ResourceAccessFlags(barrier.accessDst),
                        .LayoutBefore = ConvertToDX12ResourceLayout(barrier.layoutSrc),
                        .LayoutAfter = ConvertToDX12ResourceLayout(barrier.layoutDst),
                        .pResource = dx12Texture->GetResource(),
                        .Subresources = subresourceRange,
                        .Flags = barrier.discard ? D3D12_TEXTURE_BARRIER_FLAG_DISCARD : D3D12_TEXTURE_BARRIER_FLAG_NONE,
                    });

                    CUBE_DX12_BOUND_OBJECT(barrier.texture);
                    break;
                }
                default:
                    NOT_IMPLEMENTED();
                }
            }

            const D3D12_BARRIER_GROUP barrierGroups[2] = {
                {
                    .Type = D3D12_BARRIER_TYPE_BUFFER,
                    .NumBarriers = static_cast<UINT32>(bufferBarriers.size()),
                    .pBufferBarriers = bufferBarriers.data(),
                },
                {
                    .Type = D3D12_BARRIER_TYPE_TEXTURE,
                    .NumBarriers = static_cast<UINT32>(textureBarriers.size()),
                    .pTextureBarriers = textureBarriers.data(),
                },
            };
            if (!bufferBarriers.empty() || !textureBarriers.empty())
            {
                mCommandList->Barrier(2, barrierGroups);
            }
        }

        void DX12CommandList::SetComputePipeline(SharedPtr<ComputePipeline> computePipeline)
        {
            CHECK(IsWriting());
            CHECK(!IsInRenderPass());

            DX12ComputePipeline* dx12ComputePipeline = dynamic_cast<DX12ComputePipeline*>(computePipeline.get());
            CHECK(dx12ComputePipeline);

            mCommandList->SetPipelineState(dx12ComputePipeline->GetPipelineState());
            mComputeThreadGroupSizeX = dx12ComputePipeline->GetThreadGroupSizeX();
            mComputeThreadGroupSizeY = dx12ComputePipeline->GetThreadGroupSizeY();
            mComputeThreadGroupSizeZ = dx12ComputePipeline->GetThreadGroupSizeZ();

            mCommandListState.computePipelineState = dx12ComputePipeline->GetPipelineState();

            CUBE_DX12_BOUND_OBJECT(computePipeline);
        }

        void DX12CommandList::DispatchThreads(Uint32 numThreadsX, Uint32 numThreadsY, Uint32 numThreadsZ)
        {
            CHECK(IsWriting());
            CHECK(mType == CommandListType::Direct);

            Uint32 threadGroupX = (numThreadsX + mComputeThreadGroupSizeX - 1) / mComputeThreadGroupSizeX;
            Uint32 threadGroupY = (numThreadsY + mComputeThreadGroupSizeY - 1) / mComputeThreadGroupSizeY;
            Uint32 threadGroupZ = (numThreadsZ + mComputeThreadGroupSizeZ - 1) / mComputeThreadGroupSizeZ;

            mCommandList->Dispatch(threadGroupX, threadGroupY, threadGroupZ);
        }

        void DX12CommandList::CopyTexture(SharedPtr<Texture> srcTexture, SharedPtr<Texture> dstTexture)
        {
            // Currently support only entire copy textures. Implement other version if needed.
            CHECK(IsWriting());

            DX12Texture* dx12SrcTexture = dynamic_cast<DX12Texture*>(srcTexture.get());
            DX12Texture* dx12DstTexture = dynamic_cast<DX12Texture*>(dstTexture.get());
            CHECK(dx12SrcTexture);
            CHECK(dx12DstTexture);

            mCommandList->CopyResource(dx12DstTexture->GetResource(), dx12SrcTexture->GetResource());

            CUBE_DX12_BOUND_OBJECT(srcTexture);
            CUBE_DX12_BOUND_OBJECT(dstTexture);
        }

        void DX12CommandList::CopyBuffer(SharedPtr<Buffer> srcBuffer, Uint64 srcOffset, SharedPtr<Buffer> dstBuffer, Uint64 dstOffset, Uint64 size)
        {
            CHECK(IsWriting());

            DX12Buffer* dx12SrcBuffer = dynamic_cast<DX12Buffer*>(srcBuffer.get());
            DX12Buffer* dx12DstBuffer = dynamic_cast<DX12Buffer*>(dstBuffer.get());
            CHECK(dx12SrcBuffer);
            CHECK(dx12DstBuffer);

            mCommandList->CopyBufferRegion(dx12DstBuffer->GetResource(), dstOffset, dx12SrcBuffer->GetResource(), srcOffset, size);

            CUBE_DX12_BOUND_OBJECT(srcBuffer);
            CUBE_DX12_BOUND_OBJECT(dstBuffer);
        }

        void DX12CommandList::CopyBufferToTexture(SharedPtr<Buffer> srcBuffer, Uint64 srcOffset, SharedPtr<Texture> dstTexture)
        {
            CHECK(IsWriting());

            DX12Buffer* dx12SrcBuffer = dynamic_cast<DX12Buffer*>(srcBuffer.get());
            DX12Texture* dx12DstTexture = dynamic_cast<DX12Texture*>(dstTexture.get());
            CHECK(dx12SrcBuffer);
            CHECK(dx12DstTexture);

            ConstArrayView<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> footprints = dx12DstTexture->GetFootprints();
            const int numSubresources = static_cast<int>(footprints.size());
            for (int i = 0; i < numSubresources; ++i)
            {
                D3D12_PLACED_SUBRESOURCE_FOOTPRINT srcFootprint = footprints[i];
                srcFootprint.Offset += srcOffset;

                D3D12_TEXTURE_COPY_LOCATION srcLocation = {
                    .pResource = dx12SrcBuffer->GetResource(),
                    .Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT,
                    .PlacedFootprint = srcFootprint
                };
                D3D12_TEXTURE_COPY_LOCATION dstLocation = {
                    .pResource = dx12DstTexture->GetResource(),
                    .Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX,
                    .SubresourceIndex = (UINT)i
                };
                mCommandList->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr);
            }

            CUBE_DX12_BOUND_OBJECT(srcBuffer);
            CUBE_DX12_BOUND_OBJECT(dstTexture);
        }

        void DX12CommandList::OptimizeTextureContentsForGPUAccess(SharedPtr<Texture> texture)
        {
            // No-op on DX12.
        }

        void DX12CommandList::BeginTimestamp(StringView name)
        {
            CHECK(IsWriting());
            CHECK(mType == CommandListType::Direct);

            DX12QueryManager& queryManager = mDevice.GetQueryManager();
            
            const Uint32 beginQueryIndex = queryManager.GetCurrentLastQueryIndexAndUse(1);
            mCommandList->EndQuery(queryManager.GetCurrentTimestampHeap(), D3D12_QUERY_TYPE_TIMESTAMP, beginQueryIndex);

            mTimestampStack.push_back({
                .name = { name.begin(), name.end() },
                .beginQueryIndex = beginQueryIndex
            });

            mHasQuery = true;
        }

        void DX12CommandList::EndTimestamp()
        {
            CHECK(IsWriting());
            CHECK(mType == CommandListType::Direct);
            CHECK(!mTimestampStack.empty());

            DX12QueryManager& queryManager = mDevice.GetQueryManager();

            const Uint32 endQueryIndex = queryManager.GetCurrentLastQueryIndexAndUse(1);
            mCommandList->EndQuery(queryManager.GetCurrentTimestampHeap(), D3D12_QUERY_TYPE_TIMESTAMP, endQueryIndex);

            const TimestampBegin& topTimestamp = mTimestampStack.back();
            queryManager.AddTimestampRange(topTimestamp.name, topTimestamp.beginQueryIndex, endQueryIndex);
            mTimestampStack.pop_back();

            mHasQuery = true;
        }

        void DX12CommandList::WaitForFence(SharedPtr<Fence> fence, Uint64 fenceValue)
        {
            CHECK(IsWriting());
            CHECK(!mIsInRenderPass);

            DX12Fence* dx12Fence = dynamic_cast<DX12Fence*>(fence.get());
            CHECK(dx12Fence);

            CHECK_HR(mCommandList->Close());
            
            mSubmitActions.push_back({
                .type = SubmitAction::Type::Execute,
                .commandList = mCommandList
            });
            mSubmitActions.push_back({
                .type = SubmitAction::Type::Wait,
                .fence = dx12Fence,
                .fenceValue = fenceValue
            });

            InitCommandList(true);

            CUBE_DX12_BOUND_OBJECT(fence);
        }

        void DX12CommandList::SignalToFence(SharedPtr<Fence> fence, Uint64 fenceValue)
        {
            CHECK(IsWriting());
            CHECK(!mIsInRenderPass);
            
            DX12Fence* dx12Fence = dynamic_cast<DX12Fence*>(fence.get());
            CHECK(dx12Fence);

            CHECK_HR(mCommandList->Close());
            
            mSubmitActions.push_back({
                .type = SubmitAction::Type::Execute,
                .commandList = mCommandList
            });
            mSubmitActions.push_back({
                .type = SubmitAction::Type::Signal,
                .fence = dx12Fence,
                .fenceValue = fenceValue
            });

            InitCommandList(true);

            CUBE_DX12_BOUND_OBJECT(fence);
        }

        void DX12CommandList::Submit()
        {
            CHECK(mPhase == Phase::Closed);

            ID3D12CommandQueue* queue = (mType == CommandListType::Copy)
                ? mDevice.GetQueueManager().GetCopyQueue()
                : mDevice.GetQueueManager().GetMainQueue();

            mSubmitActions.push_back({ .type = SubmitAction::Type::Execute, .commandList = mCommandList });

            for (const SubmitAction& action : mSubmitActions)
            {
                switch (action.type)
                {
                case SubmitAction::Type::Execute:
                {
                    ID3D12CommandList* commandLists[] = { action.commandList.Get() };
                    queue->ExecuteCommandLists(1, commandLists);
                    break;
                }
                case SubmitAction::Type::Wait:
                {
                    action.fence->WaitOnQueue(queue, action.fenceValue);
                    break;
                }
                case SubmitAction::Type::Signal:
                {
                    action.fence->SignalToQueue(queue, action.fenceValue);
                    break;
                }
                default:
                    NOT_IMPLEMENTED();
                }
            }

            if (mType == CommandListType::Copy)
            {
                mDevice.GetCommandListManager().AddCopyBoundObjects(mBoundObjects);
                mDevice.GetCommandListManager().SignalCopyFence();
                mCurrentCopyAllocator = nullptr;
            }
            else
            {
                mDevice.GetCommandListManager().AddBoundObjects(mBoundObjects);
            }
            mBoundObjects.clear();
            mSubmitActions.clear();
        }

        ID3D12CommandAllocator* DX12CommandList::GetCurrentAllocator() const
        {
            if (mType == CommandListType::Copy)
            {
                return mCurrentCopyAllocator;
            }
            return mDevice.GetCommandListManager().GetCurrentAllocator();
        }

        void DX12CommandList::InitCommandList(bool createCommandList)
        {
            // TODO: Defer creating command list until it actually used.
            if (createCommandList)
            {
                D3D12_COMMAND_LIST_TYPE commandListType = mType == CommandListType::Copy
                    ? D3D12_COMMAND_LIST_TYPE_COPY
                    : D3D12_COMMAND_LIST_TYPE_DIRECT;

                ComPtr<ID3D12GraphicsCommandList7> commandList;
                CHECK_HR(mDevice.GetDevice()->CreateCommandList(0, commandListType, GetCurrentAllocator(), nullptr, IID_PPV_ARGS(&commandList)));

                mCommandList = commandList;
            }

            CHECK(mCommandList);

            if (mType == CommandListType::Direct)
            {
                ArrayView<ID3D12DescriptorHeap*> heaps = mDevice.GetDescriptorManager().GetD3D12ShaderVisibleHeaps();
                mCommandList->SetDescriptorHeaps(heaps.size(), heaps.data());
                mCommandList->SetGraphicsRootSignature(mDevice.GetShaderParameterHelper().GetRootSignature());
                mCommandList->SetComputeRootSignature(mDevice.GetShaderParameterHelper().GetRootSignature());
            }

            mPhase = Phase::Writing;
            ApplyCommandListState();
        }

        void DX12CommandList::ApplyCommandListState()
        {
            if (!mCommandListState.viewports.empty())
            {
                mCommandList->RSSetViewports(mCommandListState.viewports.size(), mCommandListState.viewports.data());
            }
            if (!mCommandListState.scissors.empty())
            {
                mCommandList->RSSetScissorRects(mCommandListState.scissors.size(), mCommandListState.scissors.data());
            }
            if (mCommandListState.primitiveTopology != D3D_PRIMITIVE_TOPOLOGY_UNDEFINED)
            {
                mCommandList->IASetPrimitiveTopology(mCommandListState.primitiveTopology);
            }
            if (mCommandListState.computePipelineState)
            {
                mCommandList->SetPipelineState(mCommandListState.computePipelineState);
            }

            DX12ShaderParameterHelper& shaderParameterHelper = mDevice.GetShaderParameterHelper();
            for (auto& [index, constantBuffer] : mCommandListState.constantBuffers)
            {
                const DX12BufferSRV* dx12SRV = dynamic_cast<DX12BufferSRV*>(constantBuffer.buffer.get());
                CHECK(dx12SRV);
                D3D12_GPU_VIRTUAL_ADDRESS gpuAddress = dx12SRV->GetGPUAddress();

                mCommandList->SetGraphicsRootConstantBufferView(index * shaderParameterHelper.GetMaxNumRegister(), gpuAddress);
                mCommandList->SetComputeRootConstantBufferView(index * shaderParameterHelper.GetMaxNumRegister(), gpuAddress);

                constantBuffer.isSet = true;
            }
        }

        void DX12CommandList::ProcessBeforeEnd()
        {
            for (SharedPtr<DX12APIObject>& boundObj : mBoundObjects)
            {
                if (DX12Buffer* dx12Buffer = dynamic_cast<DX12Buffer*>(boundObj.get()))
                {
                    if (dx12Buffer->GetUsage() == ResourceUsage::GPUtoCPU)
                    {
                        dx12Buffer->CopyToReadbackBuffer(mCommandList.Get());
                    }
                }
            }
        }
    } // namespace gapi
} // namespace cube
