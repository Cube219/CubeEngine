#pragma once

#include "GAPIHeader.h"

#include "Flags.h"

namespace cube
{
    namespace gapi
    {
        class Shader;
        class ShaderVariablesLayout;

        constexpr int MAX_NUM_RENDER_TARGETS = 8;

        struct InputElement
        {
            const char* name;
            Uint32 index;
            ElementFormat format;
            Uint32 offset;
        };

        struct RasterizerState
        {
            enum class FillMode
            {
                Line,
                Solid
            };
            FillMode fillMode = FillMode::Solid;

            enum class CullMode
            {
                None,
                Front,
                Back
            };
            CullMode cullMode = CullMode::Back;

            enum class FrontFace
            {
                Clockwise,
                CounterClockwise
            };
            FrontFace frontFace = FrontFace::CounterClockwise;
        };

        enum class BlendFactor
        {
            Zero,
            One,
            SourceColor,
            InverseSourceColor,
            SourceAlpha,
            InverseSourceAlpha,
            DestinationColor,
            InverseDestinationColor,
            DestinationAlpha,
            InverseDestinationAlpha,
            Constant,
            InverseConstant,
            Source1Color,
            InverseSource1Color,
            Source1Alpha,
            InverseSource1Alpha
        };
        enum class BlendOperator
        {
            Add,
            Subtract,
            ReverseSubtract,
            Min,
            Max
        };

        struct BlendState
        {
            bool enableBlend = false;

            BlendFactor colorSrcBlend = BlendFactor::One;
            BlendFactor colorDstBlend = BlendFactor::Zero;
            BlendOperator colorBlendOp = BlendOperator::Add;

            BlendFactor alphaSrcBlend = BlendFactor::One;
            BlendFactor alphaDstBlend = BlendFactor::Zero;
            BlendOperator alphaBlendOp = BlendOperator::Add;

            ColorMaskFlags colorWriteMask = ColorMaskFlag::RGBA;
        };

        enum class CompareFunction
        {
            Less,
            LessEqual,
            Greater,
            GreaterEqual,
            Equal,
            NotEqual,
            Never,
            Always
        };

        enum class StencilOperator
        {
            Keep,
            Zero,
            Replace,
            Invert,
            IncrementAndClamp,
            DecrementAndClamp,
            IncrementAndWrap,
            DecrementAndWrap
        };

        struct StencilDesc
        {
            CompareFunction function;
            StencilOperator failOp;
            StencilOperator depthFailOp;
            StencilOperator passOp;
        };

        struct DepthStencilState
        {
            bool enableDepth = false;
            CompareFunction depthFunction = CompareFunction::Less;

            bool enableStencil = false;
            Uint8 stencilReadMask = 0xff;
            Uint8 stencilWriteMask = 0xff;
            StencilDesc stencilFrontFaceDesc;
            StencilDesc stencilBackFaceDesc;
        };

        enum class PrimitiveTopologyType
        {
            Undefined,
            Point,
            Line,
            Triangle,
            Patch
        };

        struct GraphicsPipelineCreateInfo
        {
            SharedPtr<Shader> vertexShader = nullptr;
            SharedPtr<Shader> pixelShader = nullptr;

            const InputElement* inputLayout;
            Uint32 numInputLayoutElements;

            RasterizerState rasterizerState;

            BlendState blendStates[MAX_NUM_RENDER_TARGETS];

            DepthStencilState depthStencilState;

            PrimitiveTopologyType primitiveTopologyType = PrimitiveTopologyType::Triangle;

            Uint32 numRenderTargets;
            ElementFormat renderTargetFormats[MAX_NUM_RENDER_TARGETS];
            ElementFormat depthStencilFormat = ElementFormat::D32_Float;

            SharedPtr<ShaderVariablesLayout> shaderVariablesLayout;

            const char* debugName = "Unknown";
        };

        struct ComputePipelineCreateInfo
        {
            const char* debugName = "Unknown";
        };

        class Pipeline
        {
        public:
            Pipeline() = default;
            virtual ~Pipeline() = default;
        };
    } // namespace gapi
} // namespace cube
