#include "GAPI_DX12Pipeline.h"

#include "Allocator/FrameAllocator.h"

#include "DX12Device.h"
#include "DX12Types.h"
#include "GAPI_DX12Shader.h"
#include "GAPI_DX12ShaderVariable.h"

namespace cube
{
    namespace gapi
    {
        D3D12_INPUT_ELEMENT_DESC ConvertToDX12InputElementDesc(InputElement inputElement)
        {
            return D3D12_INPUT_ELEMENT_DESC{
                .SemanticName = inputElement.name,
                .SemanticIndex = inputElement.index,
                .Format = GetDX12ElementFormatInfo(inputElement.format).format,
                .InputSlot = 0,
                .AlignedByteOffset = inputElement.offset,
                .InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
                .InstanceDataStepRate = 0
            };
        }

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
            FrameVector<D3D12_INPUT_ELEMENT_DESC> inputElements(info.inputLayouts.size());
            for (int i = 0; i < inputElements.size(); ++i)
            {
                inputElements[i] = ConvertToDX12InputElementDesc(info.inputLayouts[i]);
            }
            
            D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPSODesc = {};
            graphicsPSODesc.InputLayout = { inputElements.data(), (Uint32)(inputElements.size()) };
            graphicsPSODesc.pRootSignature = dynamic_cast<DX12ShaderVariablesLayout*>(info.shaderVariablesLayout.get())->GetRootSignature();

            CUBE_DX12_BOUND_OBJECT(info.shaderVariablesLayout);

            if (info.vertexShader)
            {
                DX12Shader* dx12VertexShader = dynamic_cast<DX12Shader*>(info.vertexShader.get());
                graphicsPSODesc.VS.pShaderBytecode = dx12VertexShader->GetShader().GetData();
                graphicsPSODesc.VS.BytecodeLength = dx12VertexShader->GetShader().GetSize();

                CUBE_DX12_BOUND_OBJECT(info.vertexShader);
            }
            if (info.pixelShader)
            {
                DX12Shader* dx12PixelShader = dynamic_cast<DX12Shader*>(info.pixelShader.get());
                graphicsPSODesc.PS.pShaderBytecode = dx12PixelShader->GetShader().GetData();
                graphicsPSODesc.PS.BytecodeLength = dx12PixelShader->GetShader().GetSize();

                CUBE_DX12_BOUND_OBJECT(info.pixelShader);
            }

            graphicsPSODesc.RasterizerState = ConvertToDX12RasterizerDesc(info.rasterizerState);
            graphicsPSODesc.BlendState = ConvertToDX12BlendDesc(info.blendStates);
            graphicsPSODesc.DepthStencilState = ConvertToDX12DepthStencilDesc(info.depthStencilState);
            graphicsPSODesc.SampleMask = UINT_MAX;
            graphicsPSODesc.PrimitiveTopologyType = ConvertToDX12PrimitiveTopologyType(info.primitiveTopologyType);
            graphicsPSODesc.NumRenderTargets = info.numRenderTargets;
            for (int i = 0; i < info.numRenderTargets; ++i)
            {
                graphicsPSODesc.RTVFormats[i] = GetDX12ElementFormatInfo(info.renderTargetFormats[i]).format;
            }
            graphicsPSODesc.DSVFormat = GetDX12ElementFormatInfo(info.depthStencilFormat).format;
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
            D3D12_COMPUTE_PIPELINE_STATE_DESC computePSODesc = {};
            computePSODesc.pRootSignature = dynamic_cast<DX12ShaderVariablesLayout*>(info.shaderVariablesLayout.get())->GetRootSignature();
            CUBE_DX12_BOUND_OBJECT(info.shaderVariablesLayout);

            DX12Shader* dx12ComputeShader = dynamic_cast<DX12Shader*>(info.shader.get());
            CHECK(dx12ComputeShader);
            computePSODesc.CS.pShaderBytecode = dx12ComputeShader->GetShader().GetData();
            computePSODesc.CS.BytecodeLength = dx12ComputeShader->GetShader().GetSize();
            CUBE_DX12_BOUND_OBJECT(info.shader);

            CHECK_HR(device.GetDevice()->CreateComputePipelineState(&computePSODesc, IID_PPV_ARGS(&mPipelineState)));
            SET_DEBUG_NAME(mPipelineState, info.debugName);
        }

        DX12ComputePipeline::~DX12ComputePipeline()
        {
            mPipelineState = nullptr;
        }
    } // namespace gapi
} // namespace cube
