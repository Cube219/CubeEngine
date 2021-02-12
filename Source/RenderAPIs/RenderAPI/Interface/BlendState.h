#pragma once

#include "../RenderAPIHeader.h"

namespace cube
{
    namespace rapi
    {
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

        enum class ColorWriteMaskFlag
        {
            Red = 1,
            Green = 2,
            Blue = 4,
            Alpha = 8,
            None = 0,
            RGB = Red | Green | Blue,
            RGBA = Red | Green | Blue | Alpha,
            RG = Red | Green,
            BA = Blue | Alpha
        };
        using ColorWriteMaskFlags = Flags<ColorWriteMaskFlag>;
        FLAGS_OPERATOR(ColorWriteMaskFlag);

        struct BlendState
        {
            bool enableBlend = false;

            BlendFactor colorSrcBlend = BlendFactor::One;
            BlendFactor colorDstBlend = BlendFactor::Zero;
            BlendOperator colorBlendOp = BlendOperator::Add;

            BlendFactor alphaSrcBlend = BlendFactor::One;
            BlendFactor alphaDstBlend = BlendFactor::Zero;
            BlendOperator alphaBlendOp = BlendOperator::Add;

            ColorWriteMaskFlags colorWriteMask = ColorWriteMaskFlag::RGBA;
        };
    } // namespace rapi
} // namespace cube
