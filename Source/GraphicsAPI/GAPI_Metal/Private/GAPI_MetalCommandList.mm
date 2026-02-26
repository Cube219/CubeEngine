#include "GAPI_MetalCommandList.h"

#include "Allocator/FrameAllocator.h"
#include "Checker.h"
#include "GAPI_MetalBuffer.h"
#include "GAPI_MetalPipeline.h"
#include "GAPI_MetalSampler.h"
#include "GAPI_MetalTexture.h"
#include "MacOS/MacOSString.h"
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

        void MetalEncoderState::SetViewports(ArrayView<Viewport> newViewports)
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

        void MetalEncoderState::ApplyViewports(id<MTLRenderCommandEncoder> encoder)
        {
            if (!viewports.empty())
            {
                [encoder setViewports:viewports.data() count:viewports.size()];
            }
        }

        void MetalEncoderState::SetScissors(ArrayView<ScissorRect> newScissors)
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

        void MetalEncoderState::ApplyScissors(id<MTLRenderCommandEncoder> encoder)
        {
            if (!scissors.empty())
            {
                [encoder setScissorRects:scissors.data() count:scissors.size()];
            }
        }

        void MetalEncoderState::SetPrimitiveTopology(PrimitiveTopology newPrimitiveTopology)
        {
            primitiveType = ConvertToMTLPrimitiveType(newPrimitiveTopology);
        }

        void MetalEncoderState::SetConstantBuffer(SharedPtr<Buffer> buffer, Uint32 index)
        {
            MetalBuffer* metalBuffer = dynamic_cast<MetalBuffer*>(buffer.get());
            CHECK(metalBuffer);
            CHECK(metalBuffer->GetType() == BufferType::Constant);

            ConstantBuffer& constantBuffer = constantBuffers[index];
            constantBuffer.buffer = metalBuffer->GetMTLBuffer();
            constantBuffer.isSet = false;
        }

        void MetalEncoderState::ApplyConstantBuffers(id<MTLRenderCommandEncoder> encoder, bool forceAll)
        {
            for (auto& [index, constantBuffer] : constantBuffers)
            {
                if (forceAll || !constantBuffer.isSet)
                {
                    [encoder setVertexBuffer:constantBuffer.buffer offset:0 atIndex:index];
                    [encoder setFragmentBuffer:constantBuffer.buffer offset:0 atIndex:index];
                    constantBuffer.isSet = true;
                }
            }
        }

        void MetalEncoderState::ApplyConstantBuffers(id<MTLComputeCommandEncoder> computeEncoder, bool forceAll)
        {
            for (auto& [index, constantBuffer] : constantBuffers)
            {
                if (forceAll || !constantBuffer.isSet)
                {
                    [computeEncoder setBuffer:constantBuffer.buffer offset:0 atIndex:index];
                    constantBuffer.isSet = true;
                }
            }
        }

        void MetalEncoderState::ApplyAll(id<MTLRenderCommandEncoder> encoder)
        {
            ApplyViewports(encoder);
            ApplyScissors(encoder);
            ApplyConstantBuffers(encoder, true);
        }

        void MetalEncoderState::ApplyAll(id<MTLComputeCommandEncoder> encoder)
        {
            ApplyConstantBuffers(encoder, true);
        }

        MetalCommandList::MetalCommandList(const CommandListCreateInfo& info, MetalDevice& device)
            : mTimestampManager(device.GetTimestampManager())
            , mIsWriting(false)
            , mRenderEncoder(nil)
            , mComputeEncoder(nil)
        {
            mCommandQueueRef = device.GetMainCommandQueue();
            AllocateNewCommandBuffer();

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

            mCurrentEncoderState.Clear();
            mRenderEncoder = nil;
            mComputeEncoder = nil;
            mIndexBuffer = nil;
            mIndexBufferOffset = 0;
            mComputeThreadGroupSize = MTLSizeMake(0, 0, 0);

            mIsWriting = true;
        }

        void MetalCommandList::End()
        {
            CHECK(IsWriting());

            EndEncoding();
            mCurrentEncoderState.Clear();

            mIsWriting = false;
        }

        void MetalCommandList::Reset()
        {
            CHECK(!IsWriting());

            AllocateNewCommandBuffer();
        }

        void MetalCommandList::SetViewports(ArrayView<Viewport> viewports)
        {
            CHECK(IsWriting());

            mCurrentEncoderState.SetViewports(viewports);
            if (IsInRenderPass())
            {
                mCurrentEncoderState.ApplyViewports(mRenderEncoder);
            }
        }

        void MetalCommandList::SetScissors(ArrayView<ScissorRect> scissors)
        {
            CHECK(IsWriting());

            mCurrentEncoderState.SetScissors(scissors);
            if (IsInRenderPass())
            {
                mCurrentEncoderState.ApplyScissors(mRenderEncoder);
            }
        }

        void MetalCommandList::SetPrimitiveTopology(PrimitiveTopology primitiveTopology)
        {
            CHECK(IsWriting());

            mCurrentEncoderState.SetPrimitiveTopology(primitiveTopology);
        }

        void MetalCommandList::SetGraphicsPipeline(SharedPtr<GraphicsPipeline> graphicsPipeline)
        {
            CHECK(IsWriting());
            CHECK(IsInRenderPass());

            MetalGraphicsPipeline* metalGraphicsPipeline = dynamic_cast<MetalGraphicsPipeline*>(graphicsPipeline.get());
            CHECK(metalGraphicsPipeline);

            [mRenderEncoder setTriangleFillMode:metalGraphicsPipeline->GetFillMode()];
            [mRenderEncoder setCullMode:metalGraphicsPipeline->GetCullMode()];
            [mRenderEncoder setFrontFacingWinding:metalGraphicsPipeline->GetWinding()];
            [mRenderEncoder setRenderPipelineState:metalGraphicsPipeline->GetMTLRenderPipelineState()];
            [mRenderEncoder setDepthStencilState:metalGraphicsPipeline->GetMTLDepthStencilState()];
        }

        void MetalCommandList::BeginRenderPass(ArrayView<const ColorAttachment> colors, DepthStencilAttachment depthStencil)
        {
            CHECK(IsWriting());
            CHECK(!IsInRenderPass());

            EndEncoding();

            MTLRenderPassDescriptor* renderPassDescriptor = [[MTLRenderPassDescriptor alloc] init];

            CHECK(colors.size() <= MAX_NUM_RENDER_TARGETS);
            for (int i = 0; i < colors.size(); ++i)
            {
                MetalTextureRTV* metalRTV = dynamic_cast<MetalTextureRTV*>(colors[i].rtv.get());
                CHECK(metalRTV);

                renderPassDescriptor.colorAttachments[i].texture = metalRTV->GetMTLTexture();
                renderPassDescriptor.colorAttachments[i].loadAction = ConvertToMTLLoadAction(colors[i].loadOperation);
                renderPassDescriptor.colorAttachments[i].storeAction = ConvertToMTLStoreAction(colors[i].storeOperation);
                renderPassDescriptor.colorAttachments[i].clearColor = ConvertToMTLClearColor(colors[i].clearColor);
            }
            for (int i = (int)colors.size(); i < MAX_NUM_RENDER_TARGETS; ++i)
            {
                renderPassDescriptor.colorAttachments[i] = nil;
            }

            renderPassDescriptor.depthAttachment = nil;
            renderPassDescriptor.stencilAttachment = nil;
            if (depthStencil.dsv)
            {
                MetalTextureDSV* metalDSV = dynamic_cast<MetalTextureDSV*>(depthStencil.dsv.get());
                CHECK(metalDSV);

                renderPassDescriptor.depthAttachment.texture = metalDSV->GetMTLTexture();
                renderPassDescriptor.depthAttachment.loadAction = ConvertToMTLLoadAction(depthStencil.loadOperation);
                renderPassDescriptor.depthAttachment.storeAction = ConvertToMTLStoreAction(depthStencil.storeOperation);
                renderPassDescriptor.depthAttachment.clearDepth = depthStencil.clearDepth;
            }

            mRenderEncoder = [mCommandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
            [renderPassDescriptor release];
            mCurrentEncoderState.ApplyAll(mRenderEncoder);
        }

        void MetalCommandList::EndRenderPass()
        {
            CHECK(IsWriting());
            CHECK(IsInRenderPass());

            [mRenderEncoder endEncoding];
            mRenderEncoder = nil;

            mIndexBuffer = nil;
        }

        void MetalCommandList::BindVertexBuffers(Uint32 startIndex, ArrayView<SharedPtr<Buffer>> buffers, ArrayView<Uint32> offsets)
        {
            CHECK(IsWriting());
            CHECK(IsInRenderPass());
            CHECK(buffers.size() == offsets.size());

            FrameVector<id<MTLBuffer>> mtlBuffers(buffers.size());
            FrameVector<NSUInteger> mtlOffsets(buffers.size());
            for (int i = 0; i < buffers.size(); ++i)
            {
                MetalBuffer* metalBuffer = dynamic_cast<MetalBuffer*>(buffers[i].get());
                CHECK(metalBuffer);
                CHECK(metalBuffer->GetType() == BufferType::Vertex);

                mtlBuffers[i] = metalBuffer->GetMTLBuffer();
                mtlOffsets[i] = offsets[i];
            }

            [mRenderEncoder setVertexBuffers:mtlBuffers.data() offsets:mtlOffsets.data() withRange:NSMakeRange(MetalVertexBufferOffset, mtlBuffers.size())];
        }

        void MetalCommandList::BindIndexBuffer(SharedPtr<Buffer> buffer, Uint32 offset)
        {
            CHECK(IsWriting());
            CHECK(IsInRenderPass());
            CHECK(buffer->GetType() == BufferType::Index);

            MetalBuffer* metalBuffer = dynamic_cast<MetalBuffer*>(buffer.get());
            CHECK(metalBuffer);

            mIndexBuffer = metalBuffer->GetMTLBuffer();
            mIndexBufferOffset = offset;
        }

        void MetalCommandList::Draw(Uint32 numVertices, Uint32 baseVertex, Uint32 numInstances, Uint32 baseInstance)
        {
            CHECK(IsWriting());
            CHECK(IsInRenderPass());

            [mRenderEncoder
                drawPrimitives:mCurrentEncoderState.primitiveType
                vertexStart:baseVertex
                vertexCount:numVertices
                instanceCount:numInstances
                baseInstance:baseInstance
            ];
        }

        void MetalCommandList::DrawIndexed(Uint32 numIndices, Uint32 baseIndex, Uint32 baseVertex, Uint32 numInstances, Uint32 baseInstance)
        {
            CHECK(IsWriting());
            CHECK(IsInRenderPass());

            [mRenderEncoder
                drawIndexedPrimitives:mCurrentEncoderState.primitiveType
                indexCount:numIndices
                indexType:sizeof(Index) == 2 ? MTLIndexTypeUInt16 : MTLIndexTypeUInt32
                indexBuffer:mIndexBuffer
                indexBufferOffset:baseIndex * sizeof(Index)
                instanceCount:numInstances
                baseVertex:baseVertex
                baseInstance:baseInstance
            ];
        }

        void MetalCommandList::SetShaderVariableConstantBuffer(Uint32 index, SharedPtr<Buffer> constantBuffer)
        {
            CHECK(IsWriting());

            mCurrentEncoderState.SetConstantBuffer(constantBuffer, index);
            if (IsInRenderPass())
            {
                mCurrentEncoderState.ApplyConstantBuffers(mRenderEncoder);
            }
            else if (mComputeEncoder)
            {
                mCurrentEncoderState.ApplyConstantBuffers(mComputeEncoder);
            }
        }

        void MetalCommandList::UseResource(SharedPtr<TextureSRV> srv)
        {
            CHECK(IsWriting());

            MetalTextureSRV* metalTextureSRV = dynamic_cast<MetalTextureSRV*>(srv.get());
            CHECK(metalTextureSRV);

            id<MTLResource> resource = metalTextureSRV->GetMTLTexture();
            if (IsInRenderPass())
            {
                [mRenderEncoder
                    useResource:resource
                    usage:MTLResourceUsageRead
                    stages: MTLRenderStageVertex | MTLRenderStageFragment
                ];
            }
            else if (mComputeEncoder)
            {
                [mComputeEncoder
                    useResource:resource
                    usage:MTLResourceUsageRead
                ];
            }
        }

        void MetalCommandList::UseResource(SharedPtr<TextureUAV> uav)
        {
            CHECK(IsWriting());

            MetalTextureUAV* metalTextureUAV = dynamic_cast<MetalTextureUAV*>(uav.get());
            CHECK(metalTextureUAV);

            id<MTLResource> resource = metalTextureUAV->GetMTLTexture();
            if (IsInRenderPass())
            {
                [mRenderEncoder
                    useResource:resource
                    usage:MTLResourceUsageRead | MTLResourceUsageWrite
                    stages: MTLRenderStageVertex | MTLRenderStageFragment
                ];
            }
            else if (mComputeEncoder)
            {
                [mComputeEncoder
                    useResource:resource
                    usage:MTLResourceUsageRead | MTLResourceUsageWrite
                ];
            }
        }

        void MetalCommandList::ResourceTransition(TransitionState state)
        {
            CHECK(IsWriting());
            // Metal automatically translate resource state.
        }

        void MetalCommandList::ResourceTransition(ArrayView<TransitionState> states)
        {
            CHECK(IsWriting());
            // Metal automatically translate resource state.
        }

        void MetalCommandList::SetComputePipeline(SharedPtr<ComputePipeline> computePipeline)
        {
            CHECK(IsWriting());
            CHECK(!IsInRenderPass());

            MetalComputePipeline* metalComputePipeline = dynamic_cast<MetalComputePipeline*>(computePipeline.get());
            CHECK(metalComputePipeline);

            if (mComputeEncoder == nil)
            {
                mComputeEncoder = [mCommandBuffer computeCommandEncoder];
                mCurrentEncoderState.ApplyAll(mComputeEncoder);
            }

            [mComputeEncoder setComputePipelineState:metalComputePipeline->GetMTLComputePipelineState()];
            mComputeThreadGroupSize = metalComputePipeline->GetThreadGroupSize();
        }

        void MetalCommandList::DispatchThreads(Uint32 numThreadsX, Uint32 numThreadsY, Uint32 numThreadsZ)
        {
            CHECK(IsWriting());
            CHECK(mComputeEncoder != nil);

            [mComputeEncoder
                dispatchThreads:MTLSizeMake(numThreadsX, numThreadsY, numThreadsZ)
                threadsPerThreadgroup:mComputeThreadGroupSize
            ];
        }

        void MetalCommandList::InsertTimestamp(const String& name)
        {
            CHECK(IsWriting());

            // Currently most Apple silicon only support sampling data at stage boundary, not draw/dispatch boundary.
            // So it is not possible to insert timestamp based on command list.
            // Timestamp will be implemented after implement render pass.
        }

        void MetalCommandList::Submit()
        {
            CHECK(!IsWriting());

            EndEncoding();

            [mCommandBuffer commit];
            mCommandBuffer = nil;
        }

        void MetalCommandList::AllocateNewCommandBuffer()
        {
            CHECK(!IsWriting());

            MTLCommandBufferDescriptor* commandBufferDesc = [[MTLCommandBufferDescriptor alloc] init];
            commandBufferDesc.errorOptions = MTLCommandBufferErrorOptionEncoderExecutionStatus;
            mCommandBuffer = [mCommandQueueRef commandBufferWithDescriptor:commandBufferDesc];
            [commandBufferDesc release];

            [mCommandBuffer addCompletedHandler:^(id<MTLCommandBuffer> commandBuffer) {
                for (id<MTLFunctionLog> log in commandBuffer.logs)
                {
                    NSString* label = log.encoderLabel ? log.encoderLabel : @"Unknown Label";
                    CUBE_LOG(Error, Metal, "Fault encoder: {0}", label);
                    id<MTLFunctionLogDebugLocation> location = log.debugLocation;
                    if (location)
                    {
                        CUBE_LOG(Error, Metal, "    Location: {0}:{1}:{2}", location.functionName, location.line, location.column);
                    }
                }
            }];

            // mCommandBuffer = [mCommandQueueRef commandBuffer];
        }

        void MetalCommandList::EndEncoding()
        {
            if (IsInRenderPass())
            {
                [mRenderEncoder endEncoding];
                mRenderEncoder = nil;
            }

            if (mComputeEncoder)
            {
                [mComputeEncoder endEncoding];
                mComputeEncoder = nil;
            }
        }

    } // namespace gapi
} // namespace cube
