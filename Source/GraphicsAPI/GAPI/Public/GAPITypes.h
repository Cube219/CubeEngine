#pragma once

#include "Flags.h"

namespace cube
{
    namespace gapi
    {
        enum class ElementFormat
        {
            Unknown,

            R8_Typeless,
            R8_UInt,
            R8_SInt,
            R8_UNorm,
            R8_SNorm,

            R16_Typeless,
            R16_Float,
            R16_UInt,
            R16_SInt,
            R16_UNorm,
            R16_SNorm,

            R32_Typeless,
            R32_Float,
            R32_UInt,
            R32_SInt,

            RG8_Typeless,
            RG8_UInt,
            RG8_SInt,
            RG8_UNorm,
            RG8_SNorm,

            RG16_Typeless,
            RG16_Float,
            RG16_UInt,
            RG16_SInt,
            RG16_UNorm,
            RG16_SNorm,

            RG32_Typeless,
            RG32_Float,
            RG32_UInt,
            RG32_SInt,

            RGB32_Typeless,
            RGB32_Float,
            RGB32_UInt,
            RGB32_SInt,

            RGBA8_Typeless,
            RGBA8_UInt,
            RGBA8_SInt,
            RGBA8_UNorm,
            RGBA8_SNorm,
            RGBA8_UNorm_sRGB,

            RGBA16_Typeless,
            RGBA16_Float,
            RGBA16_UInt,
            RGBA16_SInt,
            RGBA16_UNorm,
            RGBA16_SNorm,

            RGBA32_Typeless,
            RGBA32_Float,
            RGBA32_UInt,
            RGBA32_SInt,

            RGB10A2_Typeless,
            RGB10A2_UNorm,
            RGB10A2_UInt,

            RG11B10_Float,

            RGB9E5_Exp,

            B5G6R5_UNorm,

            BGR5A1_UNorm,

            BGRA8_Typeless,
            BGRA8_UNorm,
            BGRA8_UNorm_sRGB,

            D16_UNorm,
            D24_UNorm_S8_UInt,
            D32_Float,

            BC1_Typeless,
            BC1_UNorm,
            BC1_UNorm_sRGB,

            BC2_Typeless,
            BC2_UNorm,
            BC2_UNorm_sRGB,

            BC3_Typeless,
            BC3_UNorm,
            BC3_UNorm_sRGB,

            BC4_Typeless,
            BC4_UNorm,
            BC4_SNorm,

            BC5_Typeless,
            BC5_UNorm,
            BC5_SNorm,

            BC6H_Typeless,
            BC6H_UFloat,
            BC6H_SFloat,

            BC7_Typeless,
            BC7_UNorm,
            BC7_UNorm_sRGB,

            Count
        };

        enum class ColorMaskFlag
        {
            None = 0,
            Red = 1,
            Green = 2,
            Blue = 4,
            Alpha = 8,
            RGB = Red | Green | Blue,
            RGBA = Red | Green | Blue | Alpha,
            RG = Red | Green,
            BA = Blue | Alpha
        };
        using ColorMaskFlags = Flags<ColorMaskFlag>;
        FLAGS_OPERATOR(ColorMaskFlag);

        struct VRAMStatus
        {
            Uint64 physicalCurrentUsage;
            Uint64 physicalMaximumUsage;
            Uint64 logicalCurrentUsage;
            Uint64 logicalMaximumUsage;
        };
    } // namespace gapi
} // namespace cube
