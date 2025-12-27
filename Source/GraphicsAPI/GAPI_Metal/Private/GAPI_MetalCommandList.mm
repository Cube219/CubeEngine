#include "GAPI_MetalCommandList.h"

#include "Allocator/FrameAllocator.h"
#include "Checker.h"
#include "GAPI_MetalBuffer.h"
#include "GAPI_MetalPipeline.h"
#include "GAPI_MetalTexture.h"
#include "MetalDevice.h"
#include "MetalUtility.h"
#include "Renderer/RenderTypes.h"

namespace cube
{
    namespace gapi
    {
        MTLPrimitiveType ConvertToMTLPrimitiveType(PrimitiveTopology primitiveTopology)
        {
            switch (primitiveTopology)
            {
            case PrimitiveTopology::PointList:
                return MTLPrimitiveTypePoint;
            case PrimitiveTopology::LineList:
                return MTLPrimitiveTypeLine;
            case PrimitiveTopology::LineStrip:
                return MTLPrimitiveTypeLineStrip;
            case PrimitiveTopology::TriangleList:
                return MTLPrimitiveTypeTriangle;
            case PrimitiveTopology::TriangleStrip:
                return MTLPrimitiveTypeTriangleStrip;
            default:
                NOT_IMPLEMENTED();
            }
            return MTLPrimitiveTypeTriangle;
        }

        MTLLoadAction ConvertToMTLLoadAction(LoadOperation loadOp)
        {
            switch (loadOp)
            {
            case LoadOperation::DontCare:
                return MTLLoadActionDontCare;
            case LoadOperation::Load:
                return MTLLoadActionLoad;
            case LoadOperation::Clear:
                return MTLLoadActionClear;
            default:
                NOT_IMPLEMENTED();
            }
            return MTLLoadActionLoad;
        }

        MTLStoreAction ConvertToMTLStoreAction(StoreOperation storeOp)
        {
            switch (storeOp)
            {
            case StoreOperation::DontCare:
                return MTLStoreActionDontCare;
            case StoreOperation::Store:
                return MTLStoreActionStore;
            default:
                NOT_IMPLEMENTED();
            }
            return MTLStoreActionStore;
        }

        MTLClearColor ConvertToMTLClearColor(Float4 color)
        {
            MTLClearColor mtlColor;
            mtlColor.red = color.x;
            mtlColor.green = color.y;
            mtlColor.blue = color.z;
            mtlColor.alpha = color.w;
            return mtlColor;
        }

        void MetalRenderEncoderState::SetViewports(ArrayView<Viewport> newViewports)
        {
            viewports.resize(newViewports.size());
            for (int i = 0; i < newViewports.size(); ++i)
            {
                viewports[i].width = newViewports[i].width;
                viewports[i].height = newViewports[i].height;
                viewports[i].originX = newViewports[i].x;
                viewports[i].originY = newViewports[i].y;
                viewports[i].znear = newViewports[i].minDepth;
                viewports[i].zfar = newViewports[i].maxDepth;
            }
        }

        void MetalRenderEncoderState::ApplyViewports(id<MTLRenderCommandEncoder> encoder)
        {
            if (!viewports.empty())
            {
                [encoder setViewports:viewports.data() count:viewports.size()];
            }
        }

        void MetalRenderEncoderState::SetScissors(ArrayView<ScissorRect> newScissors)
        {
            scissors.resize(newScissors.size());
            for (int i = 0; i < newScissors.size(); ++i)
            {
                scissors[i].width = newScissors[i].width;
                scissors[i].height = newScissors[i].height;
                scissors[i].x = newScissors[i].x;
                scissors[i].y = newScissors[i].y;
            }
        }

        void MetalRenderEncoderState::ApplyScissors(id<MTLRenderCommandEncoder> encoder)
        {
            if (!scissors.empty())
            {
                [encoder setScissorRects:scissors.data() count:scissors.size()];
            }
        }

        void MetalRenderEncoderState::SetPrimitiveTopology(PrimitiveTopology newPrimitiveTopology)
        {
            primitiveType = ConvertToMTLPrimitiveType(newPrimitiveTopology);
        }

        void MetalRenderEncoderState::SetGraphicsPipeline(SharedPtr<GraphicsPipeline> newGraphicsPipeline)
        {
            MetalGraphicsPipeline* metalGraphicsPipeline = dynamic_cast<MetalGraphicsPipeline*>(newGraphicsPipeline.get());
            CHECK(metalGraphicsPipeline);

            fillMode = metalGraphicsPipeline->GetFillMode();
            cullMode = metalGraphicsPipeline->GetCullMode();
            winding = metalGraphicsPipeline->GetWinding();
            renderPipelineState = metalGraphicsPipeline->GetMTLRenderPipelineState();
            depthStencilState = metalGraphicsPipeline->GetMTLDepthStencilState();
        }

        void MetalRenderEncoderState::ApplyGraphicsPipeline(id<MTLRenderCommandEncoder> encoder)
        {
            if (renderPipelineState != nil && depthStencilState != nil)
            {
                [encoder setTriangleFillMode:fillMode];
                [encoder setCullMode:cullMode];
                [encoder setFrontFacingWinding:winding];

                [encoder setRenderPipelineState:renderPipelineState];
                [encoder setDepthStencilState:depthStencilState];
            }
        }

        void MetalRenderEncoderState::SetVertexBuffers(ArrayView<SharedPtr<Buffer>> buffers, ArrayView<Uint32> offsets)
        {
            CHECK(buffers.size() == offsets.size());

            vertexBuffers.resize(buffers.size());
            vertexBufferOffsets.resize(buffers.size());
            for (int i = 0; i < buffers.size(); ++i)
            {
                MetalBuffer* metalBuffer = dynamic_cast<MetalBuffer*>(buffers[i].get());
                CHECK(metalBuffer);
                CHECK(metalBuffer->GetType() == BufferType::Vertex);

                vertexBuffers[i] = metalBuffer->GetMTLBuffer();
                vertexBufferOffsets[i] = offsets[i] + MetalVertexBufferOffset;
            }
        }

        void MetalRenderEncoderState::ApplyVertexBuffers(id<MTLRenderCommandEncoder> encoder)
        {
            if (!vertexBuffers.empty())
            {
                [encoder setVertexBuffers:vertexBuffers.data() offsets:vertexBufferOffsets.data() withRange:NSMakeRange(0, vertexBuffers.size())];
            }
        }

        void MetalRenderEncoderState::SetIndexBuffer(SharedPtr<Buffer> buffer, Uint32 offset)
        {
            MetalBuffer* metalBuffer = dynamic_cast<MetalBuffer*>(buffer.get());
            CHECK(metalBuffer);
            CHECK(metalBuffer->GetType() == BufferType::Index);

            indexBuffer = metalBuffer->GetMTLBuffer();
            indexBufferOffset = offset;
        }

        void MetalRenderEncoderState::SetConstantBuffer(SharedPtr<Buffer> buffer, Uint32 index)
        {
            MetalBuffer* metalBuffer = dynamic_cast<MetalBuffer*>(buffer.get());
            CHECK(metalBuffer);
            CHECK(metalBuffer->GetType() == BufferType::Constant);

            ConstantBuffer& constantBuffer = constantBuffers[index];
            constantBuffer.buffer = metalBuffer->GetMTLBuffer();
            constantBuffer.isSet = false;
        }

        void MetalRenderEncoderState::ApplyConstantBuffer(id<MTLRenderCommandEncoder> encoder)
        {
            for (auto& [index, constantBuffer] : constantBuffers)
            {
                if (!constantBuffer.isSet)
                {
                    [encoder setVertexBuffer:constantBuffer.buffer offset:0 atIndex:index];
                    [encoder setFragmentBuffer:constantBuffer.buffer offset:0 atIndex:index];
                    constantBuffer.isSet = true;
                }
            }
        }

        void MetalRenderEncoderState::ApplyAll(id<MTLRenderCommandEncoder> encoder)
        {
            ApplyViewports(encoder);
            ApplyScissors(encoder);
            ApplyGraphicsPipeline(encoder);
            ApplyVertexBuffers(encoder);
            ApplyConstantBuffer(encoder);
        }

        MetalCommandList::MetalCommandList(const CommandListCreateInfo& info, MetalDevice& device)
            : mIsNeededEncoderUpdating(false)
        {
            mCommandQueueRef = device.GetMainCommandQueue();
            mCommandBuffer = [device.GetMainCommandQueue() commandBuffer];

            mType = info.type;
            mDebugName = info.debugName;
        }

        MetalCommandList::~MetalCommandList()
        {
            CHECK(!IsWriting());

            mCommandBuffer = nil;
        }

        void MetalCommandList::Begin()
        {
            CHECK(!IsWriting());

            mRenderPassDescriptor = nil;
//            mRenderPassDescriptor = [[MTLRenderPassDescriptor alloc] init];
            mCurrentRenderEncoderState.Clear();

            mIsWriting = true;

            mIsNeededEncoderUpdating = true;
            UpdateEncoderIfNeeded();
        }

        void MetalCommandList::End()
        {
            CHECK(IsWriting());

            if (mRenderEncoder)
            {
                EndEncoding();
                mCurrentRenderEncoderState.Clear();
            }
            [mRenderPassDescriptor release];
            mRenderPassDescriptor = nil;

            mIsWriting = false;
        }

        void MetalCommandList::Reset()
        {
            // CHECK(!IsWriting());

            mCommandBuffer = [mCommandQueueRef commandBuffer];
        }

        void MetalCommandList::SetViewports(ArrayView<Viewport> viewports)
        {
            CHECK(IsWriting());

            mCurrentRenderEncoderState.SetViewports(viewports);
            UpdateEncoderIfNeeded();
            if (mRenderEncoder)
            {
                mCurrentRenderEncoderState.ApplyViewports(mRenderEncoder);
            }
        }

        void MetalCommandList::SetScissors(ArrayView<ScissorRect> scissors)
        {
            CHECK(IsWriting());

            mCurrentRenderEncoderState.SetScissors(scissors);
            UpdateEncoderIfNeeded();
            if (mRenderEncoder)
            {
                mCurrentRenderEncoderState.ApplyScissors(mRenderEncoder);
            }
        }

        void MetalCommandList::SetPrimitiveTopology(PrimitiveTopology primitiveTopology)
        {
            CHECK(IsWriting());

            mCurrentRenderEncoderState.SetPrimitiveTopology(primitiveTopology);
        }

        void MetalCommandList::SetGraphicsPipeline(SharedPtr<GraphicsPipeline> graphicsPipeline)
        {
            CHECK(IsWriting());

            mCurrentRenderEncoderState.SetGraphicsPipeline(graphicsPipeline);
            UpdateEncoderIfNeeded();
            if (mRenderEncoder)
            {
                mCurrentRenderEncoderState.ApplyGraphicsPipeline(mRenderEncoder);
            }
        }

        void MetalCommandList::SetRenderTargets(ArrayView<ColorAttachment> colors, DepthStencilAttachment depthStencil)
        {
            CHECK(IsWriting());

            if (!mRenderPassDescriptor)
            {
                mRenderPassDescriptor = [[MTLRenderPassDescriptor alloc] init];
            }

            CHECK(colors.size() <= MAX_NUM_RENDER_TARGETS);
            for (int i = 0; i < colors.size(); ++i)
            {
                MetalTextureRTV* metalRTV = dynamic_cast<MetalTextureRTV*>(colors[i].rtv.get());
                CHECK(metalRTV);

                mRenderPassDescriptor.colorAttachments[i].texture = metalRTV->GetMTLTexture();
                mRenderPassDescriptor.colorAttachments[i].loadAction = ConvertToMTLLoadAction(colors[i].loadOperation);
                mRenderPassDescriptor.colorAttachments[i].storeAction = ConvertToMTLStoreAction(colors[i].storeOperation);
                mRenderPassDescriptor.colorAttachments[i].clearColor = ConvertToMTLClearColor(colors[i].clearColor);
            }
            for (int i = (int)colors.size(); i < MAX_NUM_RENDER_TARGETS; ++i)
            {
                mRenderPassDescriptor.colorAttachments[i] = nil;
            }

            mRenderPassDescriptor.depthAttachment = nil;
            mRenderPassDescriptor.stencilAttachment = nil;
            if (depthStencil.dsv)
            {
                MetalTextureDSV* metalDSV = dynamic_cast<MetalTextureDSV*>(depthStencil.dsv.get());
                CHECK(metalDSV);

                mRenderPassDescriptor.depthAttachment.texture = metalDSV->GetMTLTexture();
                mRenderPassDescriptor.depthAttachment.loadAction = ConvertToMTLLoadAction(depthStencil.loadOperation);
                mRenderPassDescriptor.depthAttachment.storeAction = ConvertToMTLStoreAction(depthStencil.storeOperation);
                mRenderPassDescriptor.depthAttachment.clearDepth = depthStencil.clearDepth;
            }

            mIsNeededEncoderUpdating = true;
        }

        void MetalCommandList::BindVertexBuffers(Uint32 startIndex, ArrayView<SharedPtr<Buffer>> buffers, ArrayView<Uint32> offsets)
        {
            CHECK(IsWriting());

            mCurrentRenderEncoderState.SetVertexBuffers(buffers, offsets);
            UpdateEncoderIfNeeded();
            if (mRenderEncoder)
            {
                mCurrentRenderEncoderState.ApplyVertexBuffers(mRenderEncoder);
            }
        }

        void MetalCommandList::BindIndexBuffer(SharedPtr<Buffer> buffer, Uint32 offset)
        {
            CHECK(IsWriting());

            mCurrentRenderEncoderState.SetIndexBuffer(buffer, offset);
        }

        void MetalCommandList::Draw(Uint32 numVertices, Uint32 baseVertex, Uint32 numInstances, Uint32 baseInstance)
        {
            CHECK(IsWriting());
            CHECK_FORMAT(mRenderEncoder, "Render encoder is null. Maybe render targets are not set.");

            UpdateEncoderIfNeeded();
            [mRenderEncoder
                drawPrimitives:mCurrentRenderEncoderState.primitiveType
                   vertexStart:baseVertex
                   vertexCount:numVertices
                 instanceCount:numInstances
                  baseInstance:baseInstance
            ];
        }

        void MetalCommandList::DrawIndexed(Uint32 numIndices, Uint32 baseIndex, Uint32 baseVertex, Uint32 numInstances, Uint32 baseInstance)
        {
            CHECK(IsWriting());
            CHECK_FORMAT(mRenderEncoder, "Render encoder is null. Maybe render targets are not set.");

            UpdateEncoderIfNeeded();
            mCurrentRenderEncoderState.ApplyVertexBuffers(mRenderEncoder);
            [mRenderEncoder
                drawIndexedPrimitives:mCurrentRenderEncoderState.primitiveType
                           indexCount:numIndices
                            indexType:sizeof(Index) == 16 ? MTLIndexTypeUInt16 : MTLIndexTypeUInt32
                          indexBuffer:mCurrentRenderEncoderState.indexBuffer
                    indexBufferOffset:baseIndex
                        instanceCount:numInstances
                           baseVertex:baseVertex + 10
                         baseInstance:baseInstance
            ];
        }

        void MetalCommandList::SetShaderVariableConstantBuffer(Uint32 index, SharedPtr<Buffer> constantBuffer)
        {
            CHECK(IsWriting());

            mCurrentRenderEncoderState.SetConstantBuffer(constantBuffer, index);
            UpdateEncoderIfNeeded();
            mCurrentRenderEncoderState.ApplyConstantBuffer(mRenderEncoder);
        }

        void MetalCommandList::BindTexture(SharedPtr<Texture> texture)
        {
            CHECK(IsWriting());
            // TODO: Do nothing. This function will be removed.
        }

        void MetalCommandList::BindSampler(SharedPtr<Sampler> sampler)
        {
            CHECK(IsWriting());
            // TODO: Do nothing. This function will be removed.
        }

        void MetalCommandList::ResourceTransition(TransitionState state)
        {
        }

        void MetalCommandList::ResourceTransition(ArrayView<TransitionState> states)
        {
        }

        void MetalCommandList::SetComputePipeline(SharedPtr<ComputePipeline> computePipeline)
        {
        }

        void MetalCommandList::Dispatch(Uint32 threadGroupX, Uint32 threadGroupY, Uint32 threadGroupZ)
        {
        }

        void MetalCommandList::InsertTimestamp(const String& name)
        {
        }

        void MetalCommandList::Submit()
        {
            EndEncoding();

            [mCommandBuffer commit];
            mCommandBuffer = nil;
        }

        void MetalCommandList::BeginEncoding()
        {
            CHECK(mRenderEncoder == nil);

            switch (mType)
            {
            case CommandListType::Graphics:
                if (mRenderPassDescriptor)
                {
                    mRenderEncoder = [mCommandBuffer renderCommandEncoderWithDescriptor:mRenderPassDescriptor];
                    mCurrentRenderEncoderState.ApplyAll(mRenderEncoder);
                }
                break;
            case CommandListType::Compute:
                break;
            }
        }

        void MetalCommandList::EndEncoding()
        {
            if (mRenderEncoder)
            {
                [mRenderEncoder endEncoding];
                mRenderEncoder = nil;
            }
        }

        void MetalCommandList::UpdateEncoderIfNeeded()
        {
            if (mIsNeededEncoderUpdating)
            {
                EndEncoding();
                BeginEncoding();
            }

            mIsNeededEncoderUpdating = false;
        }
    } // namespace gapi
} // namespace cube
