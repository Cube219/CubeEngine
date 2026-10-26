#include "GAPI_DX12Pipeline.h"

#include "Allocator/FrameAllocator.h"

#include "DX12Device.h"
#include "DX12Types.h"
#include "GAPI_DX12Shader.h"
#include "Allocator/AllocatorUtility.h"

namespace cube
{
    namespace gapi
    {
        D3D12_RASTERIZER_DESC ConvertToDX12RasterizerDesc(RasterizerState rasterizerState)
        {
            D3D12_FILL_MODE fillMode = (D3D12_FILL_MODE)0;
            switch (rasterizerState.fillMode)
            {
            case RasterizerState::FillMode::Line:
                fillMode = D3D12_FILL_MODE_WIREFRAME;
                break;
            case RasterizerState::FillMode::Solid:
                fillMode = D3D12_FILL_MODE_SOLID;
                break;
            default:
                NOT_IMPLEMENTED();
            }
            D3D12_CULL_MODE cullMode = (D3D12_CULL_MODE)0;
            switch (rasterizerState.cullMode)
            {
            case RasterizerState::CullMode::None:
                cullMode = D3D12_CULL_MODE_NONE;
                break;
            case RasterizerState::CullMode::Front:
                cullMode = D3D12_CULL_MODE_FRONT;
                break;
            case RasterizerState::CullMode::Back:
                cullMode = D3D12_CULL_MODE_BACK;
                break;
            default:
                NOT_IMPLEMENTED();
            }

            return D3D12_RASTERIZER_DESC{
                .FillMode = fillMode,
                .CullMode = cullMode,
                .FrontCounterClockwise = (rasterizerState.frontFace == RasterizerState::FrontFace::CounterClockwise),
                .DepthBias = D3D12_DEFAULT_DEPTH_BIAS,
                .DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP,
                .SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS,
                .DepthClipEnable = TRUE,
                .MultisampleEnable = FALSE,
                .AntialiasedLineEnable = FALSE,
                .ForcedSampleCount = 0,
                .ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF
            };
        }

        D3D12_BLEND_DESC ConvertToDX12BlendDesc(const Array<BlendState, MAX_NUM_RENDER_TARGETS>& blendStates)
        {
            D3D12_BLEND_DESC desc;
            desc.AlphaToCoverageEnable = FALSE;
            desc.IndependentBlendEnable = FALSE;

            auto ConvertToDX12Blend = [](BlendFactor blendFactor) -> D3D12_BLEND {
                switch (blendFactor)
                {
                case BlendFactor::Zero:
                    return D3D12_BLEND_ZERO;
                case BlendFactor::One:
                    return D3D12_BLEND_ONE;
                case BlendFactor::SourceColor:
                    return D3D12_BLEND_SRC_COLOR;
                case BlendFactor::InverseSourceColor:
                    return D3D12_BLEND_INV_SRC_COLOR;
                case BlendFactor::SourceAlpha:
                    return D3D12_BLEND_SRC_ALPHA;
                case BlendFactor::InverseSourceAlpha:
                    return D3D12_BLEND_INV_SRC_ALPHA;
                case BlendFactor::DestinationColor:
                    return D3D12_BLEND_DEST_COLOR;
                case BlendFactor::InverseDestinationColor:
                    return D3D12_BLEND_INV_DEST_COLOR;
                case BlendFactor::DestinationAlpha:
                    return D3D12_BLEND_DEST_ALPHA;
                case BlendFactor::InverseDestinationAlpha:
                    return D3D12_BLEND_INV_DEST_ALPHA;
                case BlendFactor::Constant:
                    return D3D12_BLEND_BLEND_FACTOR;
                case BlendFactor::InverseConstant:
                    return D3D12_BLEND_INV_BLEND_FACTOR;
                case BlendFactor::Source1Color:
                    return D3D12_BLEND_SRC1_COLOR;
                case BlendFactor::InverseSource1Color:
                    return D3D12_BLEND_INV_SRC1_COLOR;
                case BlendFactor::Source1Alpha:
                    return D3D12_BLEND_SRC1_ALPHA;
                case BlendFactor::InverseSource1Alpha:
                    return D3D12_BLEND_INV_SRC1_ALPHA;
                default:
                    NOT_IMPLEMENTED();
                }
                return (D3D12_BLEND)0;
            };
            auto ConvertToDX12BlendOp = [](BlendOperator blendOperator) -> D3D12_BLEND_OP {
                switch (blendOperator)
                {
                case BlendOperator::Add:
                    return D3D12_BLEND_OP_ADD;
                case BlendOperator::Subtract:
                    return D3D12_BLEND_OP_SUBTRACT;
                case BlendOperator::ReverseSubtract:
                    return D3D12_BLEND_OP_REV_SUBTRACT;
                case BlendOperator::Min:
                    return D3D12_BLEND_OP_MIN;
                case BlendOperator::Max:
                    return D3D12_BLEND_OP_MAX;
                default:
                    NOT_IMPLEMENTED();
                }
                return (D3D12_BLEND_OP)0;
            };
            auto ConvertToDX12ColorWriteEnable = [](ColorMaskFlags colorMaskFlags) -> Uint8
            {
                Uint8 res = (D3D12_COLOR_WRITE_ENABLE)0;
                res |= (colorMaskFlags.IsSet(ColorMaskFlag::Red) ? D3D12_COLOR_WRITE_ENABLE_RED : 0);
                res |= (colorMaskFlags.IsSet(ColorMaskFlag::Green) ? D3D12_COLOR_WRITE_ENABLE_GREEN : 0);
                res |= (colorMaskFlags.IsSet(ColorMaskFlag::Blue) ? D3D12_COLOR_WRITE_ENABLE_BLUE : 0);
                res |= (colorMaskFlags.IsSet(ColorMaskFlag::Alpha) ? D3D12_COLOR_WRITE_ENABLE_ALPHA : 0);
                return res;
            };

            for (int i = 0; i < MAX_NUM_RENDER_TARGETS; ++i)
            {
                const BlendState& src = blendStates[i];

                desc.RenderTarget[i] = {
                    .BlendEnable = src.enableBlend,
                    .LogicOpEnable = FALSE,
                    .SrcBlend = ConvertToDX12Blend(src.colorSrcBlend),
                    .DestBlend = ConvertToDX12Blend(src.colorDstBlend),
                    .BlendOp = ConvertToDX12BlendOp(src.colorBlendOp),
                    .SrcBlendAlpha = ConvertToDX12Blend(src.alphaSrcBlend),
                    .DestBlendAlpha = ConvertToDX12Blend(src.alphaDstBlend),
                    .BlendOpAlpha = ConvertToDX12BlendOp(src.alphaBlendOp),
                    .LogicOp = D3D12_LOGIC_OP_NOOP,
                    .RenderTargetWriteMask = ConvertToDX12ColorWriteEnable(src.colorWriteMask)
                };
            }

            return desc;
        }

        D3D12_DEPTH_STENCIL_DESC ConvertToDX12DepthStencilDesc(DepthStencilState depthStencilState)
        {
            auto ConvertToDX12ComparisonFunc = [](CompareFunction compareFunction) -> D3D12_COMPARISON_FUNC {
                switch (compareFunction)
                {
                case CompareFunction::Less:
                    return D3D12_COMPARISON_FUNC_LESS;
                case CompareFunction::LessEqual:
                    return D3D12_COMPARISON_FUNC_LESS_EQUAL;
                case CompareFunction::Greater:
                    return D3D12_COMPARISON_FUNC_GREATER;
                case CompareFunction::GreaterEqual:
                    return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
                case CompareFunction::Equal:
                    return D3D12_COMPARISON_FUNC_EQUAL;
                case CompareFunction::NotEqual:
                    return D3D12_COMPARISON_FUNC_NOT_EQUAL;
                case CompareFunction::Never:
                    return D3D12_COMPARISON_FUNC_NEVER;
                case CompareFunction::Always:
                    return D3D12_COMPARISON_FUNC_ALWAYS;
                default:
                    NOT_IMPLEMENTED();
                }
                return (D3D12_COMPARISON_FUNC)0;
            };
            auto ConvertToDX12DepthStencilOPDesc = [ConvertToDX12ComparisonFunc](StencilDesc stencilDesc) -> D3D12_DEPTH_STENCILOP_DESC
            {
                auto ConvertToDX12StencilOp = [](StencilOperator stencilOperator) -> D3D12_STENCIL_OP {
                    switch (stencilOperator)
                    {
                    case StencilOperator::Keep:
                        return D3D12_STENCIL_OP_KEEP;
                    case StencilOperator::Zero:
                        return D3D12_STENCIL_OP_ZERO;
                    case StencilOperator::Replace:
                        return D3D12_STENCIL_OP_REPLACE;
                    case StencilOperator::Invert:
                        return D3D12_STENCIL_OP_INVERT;
                    case StencilOperator::IncrementAndClamp:
                        return D3D12_STENCIL_OP_INCR_SAT;
                    case StencilOperator::DecrementAndClamp:
                        return D3D12_STENCIL_OP_DECR_SAT;
                    case StencilOperator::IncrementAndWrap:
                        return D3D12_STENCIL_OP_INCR;
                    case StencilOperator::DecrementAndWrap:
                        return D3D12_STENCIL_OP_DECR;
                    default:
                        NOT_IMPLEMENTED();
                    }
                    return (D3D12_STENCIL_OP)0;
                };
                return D3D12_DEPTH_STENCILOP_DESC{
                    .StencilFailOp = ConvertToDX12StencilOp(stencilDesc.failOp),
                    .StencilDepthFailOp = ConvertToDX12StencilOp(stencilDesc.depthFailOp),
                    .StencilPassOp = ConvertToDX12StencilOp(stencilDesc.passOp),
                    .StencilFunc = ConvertToDX12ComparisonFunc(stencilDesc.function)
                };
            };

            return D3D12_DEPTH_STENCIL_DESC{
                .DepthEnable = depthStencilState.enableDepth,
                .DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL,
                .DepthFunc = ConvertToDX12ComparisonFunc(depthStencilState.depthFunction),
                .StencilEnable = depthStencilState.enableStencil,
                .StencilReadMask = depthStencilState.stencilReadMask,
                .StencilWriteMask = depthStencilState.stencilWriteMask,
                .FrontFace = ConvertToDX12DepthStencilOPDesc(depthStencilState.stencilFrontFaceDesc),
                .BackFace = ConvertToDX12DepthStencilOPDesc(depthStencilState.stencilBackFaceDesc)
            };
        }

        D3D12_PRIMITIVE_TOPOLOGY_TYPE ConvertToDX12PrimitiveTopologyType(PrimitiveTopologyType primitiveTopologyType)
        {
            switch (primitiveTopologyType)
            {
            case PrimitiveTopologyType::Undefined:
                return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
            case PrimitiveTopologyType::Point:
                return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
            case PrimitiveTopologyType::Line:
                return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
            case PrimitiveTopologyType::Triangle:
                return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
            case PrimitiveTopologyType::Patch:
                return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
            default:
                NOT_IMPLEMENTED();
            }
            return (D3D12_PRIMITIVE_TOPOLOGY_TYPE)0;
        }

        DX12GraphicsPipeline::DX12GraphicsPipeline(DX12Device& device, const GraphicsPipelineCreateInfo& info)
        {
            const GraphicsPipelineInfo& pipelineInfo = info.pipelineInfo;

            D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPSODesc = {};
            graphicsPSODesc.pRootSignature = device.GetShaderParameterHelper().GetRootSignature();

            if (pipelineInfo.vertexShader)
            {
                DX12Shader* dx12VertexShader = dynamic_cast<DX12Shader*>(pipelineInfo.vertexShader.get());
                graphicsPSODesc.VS.pShaderBytecode = dx12VertexShader->GetShaderByteCode().GetData();
                graphicsPSODesc.VS.BytecodeLength = dx12VertexShader->GetShaderByteCode().GetSize();

                CUBE_DX12_BOUND_OBJECT(pipelineInfo.vertexShader);
            }
            if (pipelineInfo.pixelShader)
            {
                DX12Shader* dx12PixelShader = dynamic_cast<DX12Shader*>(pipelineInfo.pixelShader.get());
                graphicsPSODesc.PS.pShaderBytecode = dx12PixelShader->GetShaderByteCode().GetData();
                graphicsPSODesc.PS.BytecodeLength = dx12PixelShader->GetShaderByteCode().GetSize();

                CUBE_DX12_BOUND_OBJECT(pipelineInfo.pixelShader);
            }

            graphicsPSODesc.RasterizerState = ConvertToDX12RasterizerDesc(pipelineInfo.rasterizerState);
            graphicsPSODesc.BlendState = ConvertToDX12BlendDesc(pipelineInfo.blendStates);
            graphicsPSODesc.DepthStencilState = ConvertToDX12DepthStencilDesc(pipelineInfo.depthStencilState);
            graphicsPSODesc.SampleMask = UINT_MAX;
            graphicsPSODesc.PrimitiveTopologyType = ConvertToDX12PrimitiveTopologyType(pipelineInfo.primitiveTopologyType);
            graphicsPSODesc.NumRenderTargets = pipelineInfo.numRenderTargets;
            for (int i = 0; i < pipelineInfo.numRenderTargets; ++i)
            {
                graphicsPSODesc.RTVFormats[i] = GetDX12ElementFormatInfo(pipelineInfo.renderTargetFormats[i]).format;
            }
            graphicsPSODesc.DSVFormat = GetDX12ElementFormatInfo(pipelineInfo.depthStencilFormat).format;
            graphicsPSODesc.SampleDesc.Count = 1;

            CHECK_HR(device.GetDevice()->CreateGraphicsPipelineState(&graphicsPSODesc, IID_PPV_ARGS(&mPipelineState)));
            SET_DEBUG_NAME(mPipelineState, info.debugName);
        }

        DX12GraphicsPipeline::~DX12GraphicsPipeline()
        {
            mPipelineState = nullptr;
        }

        DX12ComputePipeline::DX12ComputePipeline(DX12Device& device, const ComputePipelineCreateInfo& info)
        {
            const ComputePipelineInfo& pipelineInfo = info.pipelineInfo;

            D3D12_COMPUTE_PIPELINE_STATE_DESC computePSODesc = {};
            computePSODesc.pRootSignature = device.GetShaderParameterHelper().GetRootSignature();

            DX12Shader* dx12ComputeShader = dynamic_cast<DX12Shader*>(pipelineInfo.shader.get());
            CHECK(dx12ComputeShader);
            computePSODesc.CS.pShaderBytecode = dx12ComputeShader->GetShaderByteCode().GetData();
            computePSODesc.CS.BytecodeLength = dx12ComputeShader->GetShaderByteCode().GetSize();
            CUBE_DX12_BOUND_OBJECT(pipelineInfo.shader);

            CHECK_HR(device.GetDevice()->CreateComputePipelineState(&computePSODesc, IID_PPV_ARGS(&mPipelineState)));
            SET_DEBUG_NAME(mPipelineState, info.debugName);

            mThreadGroupSizeX = dx12ComputeShader->GetThreadGroupSizeX();
            mThreadGroupSizeY = dx12ComputeShader->GetThreadGroupSizeY();
            mThreadGroupSizeZ = dx12ComputeShader->GetThreadGroupSizeZ();
        }

        DX12ComputePipeline::~DX12ComputePipeline()
        {
            mPipelineState = nullptr;
        }

        DX12RayTracingPipeline::DX12RayTracingPipeline(DX12Device& device, const RayTracingPipelineCreateInfo& createInfo)
            : mDevice(device)
        {
            CHECK(mDevice.IsHWRTSupported());

            const RayTracingPipelineInfo& pipelineInfo = createInfo.pipelineInfo;

            FrameAllocationScope scope;

            FrameVector<D3D12_STATE_SUBOBJECT> subobjects;

            D3D12_GLOBAL_ROOT_SIGNATURE globalRootSignature;
            globalRootSignature.pGlobalRootSignature = device.GetShaderParameterHelper().GetRootSignature();
            subobjects.push_back({
                .Type = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE,
                .pDesc = &globalRootSignature,
            });

            auto AddShader = [&](Shader* shader, FrameWindowsString& shaderName, D3D12_EXPORT_DESC& exportDesc, D3D12_DXIL_LIBRARY_DESC& libraryDesc)
            {
                DX12Shader* dx12Shader = dynamic_cast<DX12Shader*>(shader);
                CHECK(dx12Shader);
                shaderName = String_Convert<FrameWindowsString>(dx12Shader->GetReflection().name);
                exportDesc.Name = shaderName.c_str();
                exportDesc.ExportToRename = nullptr;
                exportDesc.Flags = D3D12_EXPORT_FLAG_NONE;
                libraryDesc.DXILLibrary.pShaderBytecode = dx12Shader->GetShaderByteCode().GetData();
                libraryDesc.DXILLibrary.BytecodeLength = dx12Shader->GetShaderByteCode().GetSize();
                libraryDesc.NumExports = 1;
                libraryDesc.pExports = &exportDesc;

                subobjects.push_back({
                    .Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY,
                    .pDesc = &libraryDesc,
                });
            };

            int currentShaderTableOffset = 0;

            D3D12_EXPORT_DESC rayGenExportDesc;
            D3D12_DXIL_LIBRARY_DESC rayGenLibraryDesc;
            FrameWindowsString rayGenShaderName;
            AddShader(pipelineInfo.rayGenShader.get(), rayGenShaderName, rayGenExportDesc, rayGenLibraryDesc);
            mRayGenShaderTableOffset = 0;
            currentShaderTableOffset = mRayGenShaderTableOffset + D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
            CUBE_DX12_BOUND_OBJECT(pipelineInfo.rayGenShader);

            D3D12_EXPORT_DESC closestHitExportDesc;
            D3D12_DXIL_LIBRARY_DESC closestHitLibraryDesc;
            FrameWindowsString closestHitShaderName;
            if (pipelineInfo.closestHitShader)
            {
                AddShader(pipelineInfo.closestHitShader.get(), closestHitShaderName, closestHitExportDesc, closestHitLibraryDesc);
                CUBE_DX12_BOUND_OBJECT(pipelineInfo.closestHitShader);
            }

            D3D12_EXPORT_DESC missExportDesc;
            D3D12_DXIL_LIBRARY_DESC missLibraryDesc;
            FrameWindowsString missShaderName;
            if (pipelineInfo.missShader)
            {
                AddShader(pipelineInfo.missShader.get(), missShaderName, missExportDesc, missLibraryDesc);
                mMissShaderTableOffset = Align(currentShaderTableOffset, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);
                currentShaderTableOffset = mMissShaderTableOffset + D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
                CUBE_DX12_BOUND_OBJECT(pipelineInfo.missShader);
            }

            const bool existAnyHitShader = pipelineInfo.closestHitShader != nullptr;
            D3D12_HIT_GROUP_DESC hitGroupDesc;
            FrameWindowsString hitGroupExportName;
            if (existAnyHitShader)
            {
                hitGroupExportName = Format<FrameWindowsString>(WINDOWS_T("{0}_HitGroup"), createInfo.debugName);
                hitGroupDesc.HitGroupExport = hitGroupExportName.c_str();
                hitGroupDesc.Type = D3D12_HIT_GROUP_TYPE_TRIANGLES;
                hitGroupDesc.AnyHitShaderImport = NULL;
                hitGroupDesc.ClosestHitShaderImport = pipelineInfo.closestHitShader ? closestHitShaderName.c_str() : NULL;
                hitGroupDesc.IntersectionShaderImport = NULL;
                mHitGroupShaderTableOffset = Align(currentShaderTableOffset, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);
                currentShaderTableOffset = mHitGroupShaderTableOffset + D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;

                subobjects.push_back({
                    .Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP,
                    .pDesc = &hitGroupDesc,
                });
            }

            D3D12_RAYTRACING_SHADER_CONFIG shaderConfig;
            shaderConfig.MaxPayloadSizeInBytes = pipelineInfo.maxPayloadSize;
            shaderConfig.MaxAttributeSizeInBytes = pipelineInfo.maxAttributeSize;
            subobjects.push_back({
                .Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG,
                .pDesc = &shaderConfig,
            });

            D3D12_RAYTRACING_PIPELINE_CONFIG pipelineConfig;
            pipelineConfig.MaxTraceRecursionDepth = pipelineInfo.maxRecursionDepth;
            subobjects.push_back({
                .Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG,
                .pDesc = &pipelineConfig,
            });

            D3D12_STATE_OBJECT_DESC stateObjectDesc = {};
            stateObjectDesc.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;
            stateObjectDesc.NumSubobjects = static_cast<UINT>(subobjects.size());
            stateObjectDesc.pSubobjects = subobjects.data();

            CHECK_HR(device.GetDevice()->CreateStateObject(&stateObjectDesc, IID_PPV_ARGS(&mStateObject)));
            SET_DEBUG_NAME_FORMAT(mStateObject, "{0}_RTPSO", createInfo.debugName);
            CHECK_HR(mStateObject->QueryInterface(IID_PPV_ARGS(&mStateObjectProperties)));

            D3D12_RESOURCE_DESC1 shaderTableBufferDesc = {
                .Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
                .Alignment = 0,
                .Width = static_cast<UINT64>(currentShaderTableOffset),
                .Height = 1,
                .DepthOrArraySize = 1,
                .MipLevels = 1,
                .Format = DXGI_FORMAT_UNKNOWN,
                .SampleDesc = {
                    .Count = 1,
                    .Quality = 0,
                },
                .Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
                .Flags = D3D12_RESOURCE_FLAG_NONE,
                .SamplerFeedbackMipRegion = {0, 0, 0},
            };
            mShaderTableAllocation = device.GetMemoryAllocator().Allocate(D3D12_HEAP_TYPE_UPLOAD, shaderTableBufferDesc, D3D12_BARRIER_LAYOUT_UNDEFINED, false);
            SET_DEBUG_NAME_FORMAT(mShaderTableAllocation.resource, "{0}_ShaderTable", createInfo.debugName);
            mShaderTableAllocation.Map();
            Byte* pShaderTable = static_cast<Byte*>(mShaderTableAllocation.pMapPtr);
            {
                void* rayGenShaderId = mStateObjectProperties->GetShaderIdentifier(rayGenShaderName.c_str());
                CHECK(rayGenShaderId);
                memcpy(pShaderTable + mRayGenShaderTableOffset, rayGenShaderId, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
                if (pipelineInfo.closestHitShader)
                {
                    void* hitGroupShaderId = mStateObjectProperties->GetShaderIdentifier(hitGroupExportName.c_str());
                    CHECK(hitGroupShaderId);
                    memcpy(pShaderTable + mHitGroupShaderTableOffset, hitGroupShaderId, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
                }
                if (pipelineInfo.missShader)
                {
                    void* missShaderId = mStateObjectProperties->GetShaderIdentifier(missShaderName.c_str());
                    CHECK(missShaderId);
                    memcpy(pShaderTable + mMissShaderTableOffset, missShaderId, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
                }
            }
            mShaderTableAllocation.Unmap();
        }

        DX12RayTracingPipeline::~DX12RayTracingPipeline()
        {
            mDevice.GetMemoryAllocator().Free(mShaderTableAllocation);

            mStateObjectProperties = nullptr;
            mStateObject = nullptr;
        }
    } // namespace gapi
} // namespace cube
