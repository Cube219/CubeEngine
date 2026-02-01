#include "GAPI_MetalPipeline.h"

#include "Allocator/FrameAllocator.h"
#include "Checker.h"
#include "GAPI_MetalShader.h"
#include "GAPI_Pipeline.h"
#include "MacOS/MacOSString.h"
#include "MetalDevice.h"
#include "MetalTypes.h"
#include "MetalUtility.h"

namespace cube
{
    namespace gapi
    {
        MTLTriangleFillMode ConvertToMTLTriangleFillMode(RasterizerState::FillMode fillMode)
        {
            switch (fillMode)
            {
            case RasterizerState::FillMode::Line:
                return MTLTriangleFillModeLines;
            case RasterizerState::FillMode::Solid:
                return MTLTriangleFillModeFill;
            default:
                NOT_IMPLEMENTED();
            }
            return MTLTriangleFillModeFill;
        }

        MTLCullMode ConvertToMTLCullMode(RasterizerState::CullMode cullMode)
        {
            switch (cullMode)
            {
            case RasterizerState::CullMode::None:
                return MTLCullModeNone;
            case RasterizerState::CullMode::Front:
                return MTLCullModeFront;
            case RasterizerState::CullMode::Back:
                return MTLCullModeBack;
            default:
                NOT_IMPLEMENTED();
            }
            return MTLCullModeBack;
        }

        MTLWinding ConvertToMTLWinding(RasterizerState::FrontFace frontFace)
        {
            switch (frontFace)
            {
            case RasterizerState::FrontFace::Clockwise:
                return MTLWindingClockwise;
            case RasterizerState::FrontFace::CounterClockwise:
                return MTLWindingCounterClockwise;
            default:
                NOT_IMPLEMENTED();
            }
            return MTLWindingCounterClockwise;
        }

        MTLBlendFactor ConvertToMTLBlendFactor(BlendFactor factor)
        {
            switch (factor)
            {
            case BlendFactor::Zero:
                return MTLBlendFactorZero;
            case BlendFactor::One:
                return MTLBlendFactorOne;
            case BlendFactor::SourceColor:
                return MTLBlendFactorSourceColor;
            case BlendFactor::InverseSourceColor:
                return MTLBlendFactorOneMinusSourceColor;
            case BlendFactor::SourceAlpha:
                return MTLBlendFactorSourceAlpha;
            case BlendFactor::InverseSourceAlpha:
                return MTLBlendFactorOneMinusSourceAlpha;
            case BlendFactor::DestinationColor:
                return MTLBlendFactorDestinationColor;
            case BlendFactor::InverseDestinationColor:
                return MTLBlendFactorOneMinusDestinationColor;
            case BlendFactor::DestinationAlpha:
                return MTLBlendFactorDestinationAlpha;
            case BlendFactor::InverseDestinationAlpha:
                return MTLBlendFactorOneMinusDestinationAlpha;
            case BlendFactor::Constant:
                return MTLBlendFactorBlendColor;
            case BlendFactor::InverseConstant:
                return MTLBlendFactorOneMinusBlendColor;
            case BlendFactor::Source1Color:
                return MTLBlendFactorSource1Color;
            case BlendFactor::InverseSource1Color:
                return MTLBlendFactorOneMinusSource1Color;
            case BlendFactor::Source1Alpha:
                return MTLBlendFactorSource1Alpha;
            case BlendFactor::InverseSource1Alpha:
                return MTLBlendFactorOneMinusSource1Alpha;
            default:
                NOT_IMPLEMENTED();
            }
            return MTLBlendFactorZero;
        }

        MTLBlendOperation ConvertToMTLBlendOperation(BlendOperator op)
        {
            switch (op)
            {
            case BlendOperator::Add:
                return MTLBlendOperationAdd;
            case BlendOperator::Subtract:
                return MTLBlendOperationSubtract;
            case BlendOperator::ReverseSubtract:
                return MTLBlendOperationReverseSubtract;
            case BlendOperator::Min:
                return MTLBlendOperationMin;
            case BlendOperator::Max:
                return MTLBlendOperationMax;
            default:
                NOT_IMPLEMENTED();
            }
            return MTLBlendOperationAdd;
        }

        MTLColorWriteMask ConvertToMTLColorWriteMask(ColorMaskFlags colorMaskFlags)
        {
            MTLColorWriteMask res = MTLColorWriteMaskNone;
            res |= (colorMaskFlags.IsSet(ColorMaskFlag::Red) ? MTLColorWriteMaskRed : 0);
            res |= (colorMaskFlags.IsSet(ColorMaskFlag::Green) ? MTLColorWriteMaskGreen : 0);
            res |= (colorMaskFlags.IsSet(ColorMaskFlag::Blue) ? MTLColorWriteMaskBlue : 0);
            res |= (colorMaskFlags.IsSet(ColorMaskFlag::Alpha) ? MTLColorWriteMaskAlpha : 0);
            return res;
        }

        MTLPrimitiveTopologyClass ConvertToMTLPrimitiveTopologyClass(PrimitiveTopologyType topologyType)
        {
            switch (topologyType)
            {
            case PrimitiveTopologyType::Point:
                return MTLPrimitiveTopologyClassPoint;
            case PrimitiveTopologyType::Line:
                return MTLPrimitiveTopologyClassLine;
            case PrimitiveTopologyType::Triangle:
                return MTLPrimitiveTopologyClassTriangle;
            default:
                NOT_IMPLEMENTED();
            }
            return MTLPrimitiveTopologyClassTriangle;
        }

        MTLCompareFunction ConvertToMTLCompareFunction(CompareFunction compareFunction)
        {
            switch (compareFunction)
            {
            case CompareFunction::Less:
                return MTLCompareFunctionLess;
            case CompareFunction::LessEqual:
                return MTLCompareFunctionLessEqual;
            case CompareFunction::Greater:
                return MTLCompareFunctionGreater;
            case CompareFunction::GreaterEqual:
                return MTLCompareFunctionGreaterEqual;
            case CompareFunction::Equal:
                return MTLCompareFunctionEqual;
            case CompareFunction::NotEqual:
                return MTLCompareFunctionNotEqual;
            case CompareFunction::Never:
                return MTLCompareFunctionNever;
            case CompareFunction::Always:
                return MTLCompareFunctionAlways;
            default:
                NOT_IMPLEMENTED();
            }
            return MTLCompareFunctionAlways;
        }

        MTLStencilOperation ConvertToMTLStencilOperation(StencilOperator stencilOperator)
        {
            switch (stencilOperator)
            {
            case StencilOperator::Keep:
                return MTLStencilOperationKeep;
            case StencilOperator::Zero:
                return MTLStencilOperationZero;
            case StencilOperator::Replace:
                return MTLStencilOperationReplace;
            case StencilOperator::Invert:
                return MTLStencilOperationInvert;
            case StencilOperator::IncrementAndClamp:
                return MTLStencilOperationIncrementClamp;
            case StencilOperator::DecrementAndClamp:
                return MTLStencilOperationDecrementClamp;
            case StencilOperator::IncrementAndWrap:
                return MTLStencilOperationIncrementWrap;
            case StencilOperator::DecrementAndWrap:
                return MTLStencilOperationDecrementWrap;
            default:
                NOT_IMPLEMENTED();
            }
            return MTLStencilOperationKeep;
        }

        MetalGraphicsPipeline::MetalGraphicsPipeline(const GraphicsPipelineCreateInfo& info, MetalDevice& device)
        { @autoreleasepool {
            mFillMode = ConvertToMTLTriangleFillMode(info.rasterizerState.fillMode);
            mCullMode = ConvertToMTLCullMode(info.rasterizerState.cullMode);
            mWinding = ConvertToMTLWinding(info.rasterizerState.frontFace);

            MTLVertexDescriptor* vertexDesc = [[MTLVertexDescriptor alloc] init];
            Uint64 maxStride = 0;
            for (Uint32 i = 0; i < info.inputLayouts.size(); ++i)
            {
                const InputElement& element = info.inputLayouts[i];

                MetalElementFormatInfo formatInfo = GetMetalElementFormatInfo(element.format);
                vertexDesc.attributes[i].format = formatInfo.vertexFormat;
                vertexDesc.attributes[i].offset = element.offset;
                vertexDesc.attributes[i].bufferIndex = MetalVertexBufferOffset;

                maxStride = std::max(maxStride, (Uint64)element.offset + formatInfo.bytes);
            }
            vertexDesc.layouts[MetalVertexBufferOffset].stride = maxStride;
            vertexDesc.layouts[MetalVertexBufferOffset].stepFunction = MTLVertexStepFunctionPerVertex;
            vertexDesc.layouts[MetalVertexBufferOffset].stepRate = 1;
            
            MTLRenderPipelineDescriptor* desc = [[MTLRenderPipelineDescriptor alloc] init];
            if (info.vertexShader)
            {
                MetalShader* metalVertexShader = dynamic_cast<MetalShader*>(info.vertexShader.get());
                CHECK(metalVertexShader);
                desc.vertexFunction = metalVertexShader->GetMTLFunction();
            }
            if (info.pixelShader)
            {
                MetalShader* metalPixelShader = dynamic_cast<MetalShader*>(info.pixelShader.get());
                CHECK(metalPixelShader);
                desc.fragmentFunction = metalPixelShader->GetMTLFunction();
            }
            desc.vertexDescriptor = vertexDesc;

            desc.rasterSampleCount = 1;
            desc.alphaToCoverageEnabled = false;
            desc.alphaToOneEnabled = false;
            desc.rasterizationEnabled = info.pixelShader ? true : false;

            for (Uint32 i = 0; i < info.numRenderTargets; ++i)
            {
                MetalElementFormatInfo formatInfo = GetMetalElementFormatInfo(info.renderTargetFormats[i]);
                desc.colorAttachments[i].pixelFormat = formatInfo.pixelFormat;

                const BlendState& blendState = info.blendStates[i];
                desc.colorAttachments[i].blendingEnabled = blendState.enableBlend;
                desc.colorAttachments[i].sourceRGBBlendFactor = ConvertToMTLBlendFactor(blendState.colorSrcBlend);
                desc.colorAttachments[i].destinationRGBBlendFactor = ConvertToMTLBlendFactor(blendState.colorDstBlend);
                desc.colorAttachments[i].rgbBlendOperation = ConvertToMTLBlendOperation(blendState.colorBlendOp);
                desc.colorAttachments[i].sourceAlphaBlendFactor = ConvertToMTLBlendFactor(blendState.alphaSrcBlend);
                desc.colorAttachments[i].destinationAlphaBlendFactor = ConvertToMTLBlendFactor(blendState.alphaDstBlend);
                desc.colorAttachments[i].alphaBlendOperation = ConvertToMTLBlendOperation(blendState.alphaBlendOp);
                desc.colorAttachments[i].writeMask = ConvertToMTLColorWriteMask(blendState.colorWriteMask);
            }

            desc.depthAttachmentPixelFormat = GetMetalElementFormatInfo(info.depthStencilFormat).pixelFormat;
            desc.stencilAttachmentPixelFormat = info.depthStencilState.enableStencil ? GetMetalElementFormatInfo(info.depthStencilFormat).pixelFormat : MTLPixelFormatInvalid;

            desc.inputPrimitiveTopology = ConvertToMTLPrimitiveTopologyClass(info.primitiveTopologyType);
            desc.supportIndirectCommandBuffers = false;
            desc.label = String_Convert<NSString*>(Format<FrameString>(CUBE_T("{0} (RenderPipelineState)"), info.debugName));

            NSError* error = nil;
            mPipelineState = [device.GetMTLDevice() newRenderPipelineStateWithDescriptor:desc error:&error];
            [desc release];
            [vertexDesc release];
            if (error != nil)
            {
                CHECK_FORMAT(false, "Failed to create render pipeline state. ({0})", [error localizedDescription]);
            }
            CHECK(mPipelineState);

            // Also create depth stencil descriptor in here.
            MTLDepthStencilDescriptor* depthStencilDesc = [[MTLDepthStencilDescriptor alloc] init];
            depthStencilDesc.depthCompareFunction = ConvertToMTLCompareFunction(info.depthStencilState.depthFunction);
            depthStencilDesc.depthWriteEnabled = info.depthStencilState.enableDepth;
            if (info.depthStencilState.enableStencil)
            {
                MTLStencilDescriptor* frontStencilDesc = [[MTLStencilDescriptor alloc] init];
                frontStencilDesc.stencilCompareFunction = ConvertToMTLCompareFunction(info.depthStencilState.stencilFrontFaceDesc.function);
                frontStencilDesc.stencilFailureOperation = ConvertToMTLStencilOperation(info.depthStencilState.stencilFrontFaceDesc.failOp);
                frontStencilDesc.depthFailureOperation = ConvertToMTLStencilOperation(info.depthStencilState.stencilFrontFaceDesc.depthFailOp);
                frontStencilDesc.depthStencilPassOperation = ConvertToMTLStencilOperation(info.depthStencilState.stencilFrontFaceDesc.passOp);
                frontStencilDesc.readMask = info.depthStencilState.stencilReadMask;
                frontStencilDesc.writeMask = info.depthStencilState.stencilWriteMask;
                depthStencilDesc.frontFaceStencil = frontStencilDesc;
                [frontStencilDesc release];

                MTLStencilDescriptor* backStencilDesc = [[MTLStencilDescriptor alloc] init];
                backStencilDesc.stencilCompareFunction = ConvertToMTLCompareFunction(info.depthStencilState.stencilBackFaceDesc.function);
                backStencilDesc.stencilFailureOperation = ConvertToMTLStencilOperation(info.depthStencilState.stencilBackFaceDesc.failOp);
                backStencilDesc.depthFailureOperation = ConvertToMTLStencilOperation(info.depthStencilState.stencilBackFaceDesc.depthFailOp);
                backStencilDesc.depthStencilPassOperation = ConvertToMTLStencilOperation(info.depthStencilState.stencilBackFaceDesc.passOp);
                backStencilDesc.readMask = info.depthStencilState.stencilReadMask;
                backStencilDesc.writeMask = info.depthStencilState.stencilWriteMask;
                depthStencilDesc.backFaceStencil = backStencilDesc;
                [backStencilDesc release];
            }
            depthStencilDesc.label = String_Convert<NSString*>(Format<FrameString>(CUBE_T("{0} (DepthStencilState)"), info.debugName));

            mDepthStencilState = [device.GetMTLDevice() newDepthStencilStateWithDescriptor:depthStencilDesc];
            [depthStencilDesc release];
            CHECK(mDepthStencilState);
        }}

        MetalGraphicsPipeline::~MetalGraphicsPipeline()
        {
            [mDepthStencilState release];
            [mPipelineState release];
        }

        MetalComputePipeline::MetalComputePipeline(const ComputePipelineCreateInfo& info, MetalDevice& device)
        { @autoreleasepool {
            MetalShader* metalComputeShader = dynamic_cast<MetalShader*>(info.shader.get());
            CHECK(metalComputeShader);

            MTLComputePipelineDescriptor* desc = [[MTLComputePipelineDescriptor alloc] init];

            desc.computeFunction = metalComputeShader->GetMTLFunction();
            desc.label = String_Convert<NSString*>(info.debugName);

            NSError* error = nil;
            mPipelineState = [device.GetMTLDevice() newComputePipelineStateWithDescriptor:desc options:MTLPipelineOptionNone reflection:nil error:&error];
            [desc release];

            if (error != nil)
            {
                CHECK_FORMAT(false, "Failed to create compute pipeline state. ({0})", [error localizedDescription]);
            }
            CHECK(mPipelineState);

            mThreadGroupSize = MTLSizeMake(metalComputeShader->GetThreadGroupSizeX(), metalComputeShader->GetThreadGroupSizeY(), metalComputeShader->GetThreadGroupSizeZ());
        }}

        MetalComputePipeline::~MetalComputePipeline()
        {
            [mPipelineState release];
        }
    } // namespace gapi
} // namespace cube
