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

        void MetalEncoderState::SetGraphicsPipeline(SharedPtr<GraphicsPipeline> newGraphicsPipeline)
        {
            MetalGraphicsPipeline* metalGraphicsPipeline = dynamic_cast<MetalGraphicsPipeline*>(newGraphicsPipeline.get());
            CHECK(metalGraphicsPipeline);

            fillMode = metalGraphicsPipeline->GetFillMode();
            cullMode = metalGraphicsPipeline->GetCullMode();
            winding = metalGraphicsPipeline->GetWinding();
            renderPipelineState = metalGraphicsPipeline->GetMTLRenderPipelineState();
            depthStencilState = metalGraphicsPipeline->GetMTLDepthStencilState();
        }

        void MetalEncoderState::ApplyGraphicsPipeline(id<MTLRenderCommandEncoder> encoder)
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

        void MetalEncoderState::SetVertexBuffers(ArrayView<SharedPtr<Buffer>> buffers, ArrayView<Uint32> offsets)
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
                vertexBufferOffsets[i] = offsets[i];
            }
        }

        void MetalEncoderState::ApplyVertexBuffers(id<MTLRenderCommandEncoder> encoder)
        {
            if (!vertexBuffers.empty())
            {
                [encoder setVertexBuffers:vertexBuffers.data() offsets:vertexBufferOffsets.data() withRange:NSMakeRange(MetalVertexBufferOffset, vertexBuffers.size())];
            }
        }

        void MetalEncoderState::SetIndexBuffer(SharedPtr<Buffer> buffer, Uint32 offset)
        {
            MetalBuffer* metalBuffer = dynamic_cast<MetalBuffer*>(buffer.get());
            CHECK(metalBuffer);
            CHECK(metalBuffer->GetType() == BufferType::Index);

            indexBuffer = metalBuffer->GetMTLBuffer();
            indexBufferOffset = offset;
        }

        void MetalEncoderState::SetComputePipeline(SharedPtr<ComputePipeline> newComputePipeline)
        {
            MetalComputePipeline* metalComputePipeline = dynamic_cast<MetalComputePipeline*>(newComputePipeline.get());
            CHECK(metalComputePipeline);

            computePipelineState = metalComputePipeline->GetMTLComputePipelineState();
            computeThreadGroupSize = metalComputePipeline->GetThreadGroupSize();
        }

        void MetalEncoderState::ApplyComputePipeline(id<MTLComputeCommandEncoder> encoder)
        {
            if (computePipelineState != nil)
            {
                [encoder setComputePipelineState:computePipelineState];
            }
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

        void MetalEncoderState::AddUsedResource(id<MTLResource> resource, MTLResourceUsage usage)
        {
            usedResources.push_back({ resource, usage });
        }

        void MetalEncoderState::ApplyAllUsedResource(id<MTLRenderCommandEncoder> encoder)
        {
            for (auto [resource, usage] : usedResources)
            {
                [encoder
                    useResource:resource
                    usage:usage
                    stages: MTLRenderStageVertex | MTLRenderStageFragment
                ];
            }
        }

        void MetalEncoderState::ApplyAllUsedResource(id<MTLComputeCommandEncoder> computeEncoder)
        {
            for (auto [resource, usage] : usedResources)
            {
                [computeEncoder
                    useResource:resource
                    usage:usage
                ];
            }
        }

        void MetalEncoderState::ApplyAll(id<MTLRenderCommandEncoder> encoder)
        {
            ApplyViewports(encoder);
            ApplyScissors(encoder);
            ApplyGraphicsPipeline(encoder);
            ApplyVertexBuffers(encoder);
            ApplyConstantBuffers(encoder, true);
            ApplyAllUsedResource(encoder);
        }

        void MetalEncoderState::ApplyAll(id<MTLComputeCommandEncoder> encoder)
        {
            ApplyComputePipeline(encoder);
            ApplyConstantBuffers(encoder, true);
            ApplyAllUsedResource(encoder);
        }

        MetalCommandList::MetalCommandList(const CommandListCreateInfo& info, MetalDevice& device)
            : mIsWriting(false)
            , mIsNeededRenderEncoderUpdating(false)
            , mRenderPassDescriptor(nullptr)
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

            mRenderPassDescriptor = nil;
            mCurrentEncoderState.Clear();
            mRenderEncoder = nil;
            mComputeEncoder = nil;
            mIsNeededRenderEncoderUpdating = false;

            mIsWriting = true;
        }

        void MetalCommandList::End()
        {
            CHECK(IsWriting());

            EndEncoding();
            mCurrentEncoderState.Clear();
            if (mRenderPassDescriptor)
            {
                [mRenderPassDescriptor release];
                mRenderPassDescriptor = nil;
            }

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
            UpdateEncoderIfNeeded(EncoderType::Graphics);
            if (mRenderEncoder)
            {
                mCurrentEncoderState.ApplyViewports(mRenderEncoder);
            }
        }

        void MetalCommandList::SetScissors(ArrayView<ScissorRect> scissors)
        {
            CHECK(IsWriting());

            mCurrentEncoderState.SetScissors(scissors);
            UpdateEncoderIfNeeded(EncoderType::Graphics);
            if (mRenderEncoder)
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

            mCurrentEncoderState.SetGraphicsPipeline(graphicsPipeline);
            UpdateEncoderIfNeeded(EncoderType::Graphics);
            if (mRenderEncoder)
            {
                mCurrentEncoderState.ApplyGraphicsPipeline(mRenderEncoder);
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

            mIsNeededRenderEncoderUpdating = true;
        }

        void MetalCommandList::BindVertexBuffers(Uint32 startIndex, ArrayView<SharedPtr<Buffer>> buffers, ArrayView<Uint32> offsets)
        {
            CHECK(IsWriting());

            mCurrentEncoderState.SetVertexBuffers(buffers, offsets);
            UpdateEncoderIfNeeded(EncoderType::Graphics);
            if (mRenderEncoder)
            {
                mCurrentEncoderState.ApplyVertexBuffers(mRenderEncoder);
            }
        }

        void MetalCommandList::BindIndexBuffer(SharedPtr<Buffer> buffer, Uint32 offset)
        {
            CHECK(IsWriting());

            mCurrentEncoderState.SetIndexBuffer(buffer, offset);
        }

        void MetalCommandList::Draw(Uint32 numVertices, Uint32 baseVertex, Uint32 numInstances, Uint32 baseInstance)
        {
            CHECK(IsWriting());
            CHECK_FORMAT(mRenderEncoder, "Render encoder is null. Maybe render targets are not set.");

            UpdateEncoderIfNeeded(EncoderType::Graphics);
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
            CHECK_FORMAT(mRenderEncoder, "Render encoder is null. Maybe render targets are not set.");

            UpdateEncoderIfNeeded(EncoderType::Graphics);
            mCurrentEncoderState.ApplyVertexBuffers(mRenderEncoder);
            [mRenderEncoder
                drawIndexedPrimitives:mCurrentEncoderState.primitiveType
                indexCount:numIndices
                indexType:sizeof(Index) == 2 ? MTLIndexTypeUInt16 : MTLIndexTypeUInt32
                indexBuffer:mCurrentEncoderState.indexBuffer
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
            if (mRenderEncoder)
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
            mCurrentEncoderState.AddUsedResource(resource, MTLResourceUsageRead);
            if (mRenderEncoder)
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
            mCurrentEncoderState.AddUsedResource(resource, MTLResourceUsageRead | MTLResourceUsageWrite);
            if (mRenderEncoder)
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

            mCurrentEncoderState.SetComputePipeline(computePipeline);
            UpdateEncoderIfNeeded(EncoderType::Compute);
            if (mComputeEncoder)
            {
                mCurrentEncoderState.ApplyComputePipeline(mComputeEncoder);
            }
        }

        void MetalCommandList::DispatchThreads(Uint32 numThreadsX, Uint32 numThreadsY, Uint32 numThreadsZ)
        {
            CHECK(IsWriting());

            UpdateEncoderIfNeeded(EncoderType::Compute);
            [mComputeEncoder
                dispatchThreads:MTLSizeMake(numThreadsX, numThreadsY, numThreadsZ)
                threadsPerThreadgroup:mCurrentEncoderState.computeThreadGroupSize
            ];
        }

        void MetalCommandList::InsertTimestamp(const String& name)
        {
            CHECK(IsWriting());
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

        void MetalCommandList::BeginEncoding(EncoderType type)
        {
            CHECK(mRenderEncoder == nil);
            CHECK(mComputeEncoder == nil);

            switch (type)
            {
            case EncoderType::Graphics:
                if (mRenderPassDescriptor)
                {
                    mRenderEncoder = [mCommandBuffer renderCommandEncoderWithDescriptor:mRenderPassDescriptor];
                    mCurrentEncoderState.ApplyAll(mRenderEncoder);
                }
                break;
            case EncoderType::Compute:
                mComputeEncoder = [mCommandBuffer computeCommandEncoder];
                mCurrentEncoderState.ApplyAll(mComputeEncoder);
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

            if (mComputeEncoder)
            {
                [mComputeEncoder endEncoding];
                mComputeEncoder = nil;
            }
        }

        void MetalCommandList::UpdateEncoderIfNeeded(EncoderType type)
        {
            if ((type == EncoderType::Graphics && mIsNeededRenderEncoderUpdating) ||
                (type == EncoderType::Compute && mComputeEncoder == nil))
            {
                EndEncoding();
                BeginEncoding(type);
            }

            mIsNeededRenderEncoderUpdating = false;
        }
    } // namespace gapi
} // namespace cube
