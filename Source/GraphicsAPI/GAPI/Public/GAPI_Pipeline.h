#pragma once

#include "GAPIHeader.h"

#include "CubeString.h"
#include "Flags.h"

namespace cube
{
    namespace gapi
    {
        class Shader;

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

            ArrayView<InputElement> inputLayouts;

            RasterizerState rasterizerState;

            Array<BlendState, MAX_NUM_RENDER_TARGETS> blendStates;

            DepthStencilState depthStencilState;

            PrimitiveTopologyType primitiveTopologyType = PrimitiveTopologyType::Triangle;

            Uint32 numRenderTargets;
            Array<ElementFormat, MAX_NUM_RENDER_TARGETS> renderTargetFormats;
            ElementFormat depthStencilFormat = ElementFormat::D32_Float;

            StringView debugName;
        };

        class GraphicsPipeline
        {
        public:
            GraphicsPipeline() = default;
            virtual ~GraphicsPipeline() = default;
        };

        struct ComputePipelineCreateInfo
        {
            SharedPtr<Shader> shader;
            
            StringView debugName;
        };

        class ComputePipeline
        {
        public:
            ComputePipeline() = default;
            virtual ~ComputePipeline() = default;
        };
    } // namespace gapi
} // namespace cube
