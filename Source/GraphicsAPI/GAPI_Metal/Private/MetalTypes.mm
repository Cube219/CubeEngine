#include "MetalTypes.h"

#include "Logger.h"

namespace cube
{
    MetalElementFormatInfo gMetalElementFormatInfos[(int)gapi::ElementFormat::Count] = {};

    MetalElementFormatInfo GetMetalElementFormatInfo(gapi::ElementFormat elementFormat)
    {
        return gMetalElementFormatInfos[(int)elementFormat];
    }

    void InitializeTypes()
    {
        using namespace gapi;

#define INIT_ELEMENT_FORMAT(elementFormat, inMTLPixelFormat, inBytes) \
        gMetalElementFormatInfos[(int)(elementFormat)] = { .pixelFormat = (inMTLPixelFormat), .bytes = (inBytes) }
#define INIT_ELEMENT_FORMAT_UNSUPPORT(elementFormat) \
        gMetalElementFormatInfos[(int)(elementFormat)].unsupported = true

        INIT_ELEMENT_FORMAT_UNSUPPORT(ElementFormat::Unknown);

        INIT_ELEMENT_FORMAT_UNSUPPORT(ElementFormat::R8_Typeless);
        INIT_ELEMENT_FORMAT(ElementFormat::R8_UInt, MTLPixelFormatR8Uint, 1);
        INIT_ELEMENT_FORMAT(ElementFormat::R8_SInt, MTLPixelFormatR8Sint, 1);
        INIT_ELEMENT_FORMAT(ElementFormat::R8_UNorm, MTLPixelFormatR8Unorm, 1);
        INIT_ELEMENT_FORMAT(ElementFormat::R8_SNorm, MTLPixelFormatR8Snorm, 1);

        INIT_ELEMENT_FORMAT_UNSUPPORT(ElementFormat::R16_Typeless);
        INIT_ELEMENT_FORMAT(ElementFormat::R16_Float, MTLPixelFormatR16Float, 2);
        INIT_ELEMENT_FORMAT(ElementFormat::R16_UInt, MTLPixelFormatR16Uint, 2);
        INIT_ELEMENT_FORMAT(ElementFormat::R16_SInt, MTLPixelFormatR16Sint, 2);
        INIT_ELEMENT_FORMAT(ElementFormat::R16_UNorm, MTLPixelFormatR16Unorm, 2);
        INIT_ELEMENT_FORMAT(ElementFormat::R16_SNorm, MTLPixelFormatR16Snorm, 2);

        INIT_ELEMENT_FORMAT_UNSUPPORT(ElementFormat::R32_Typeless);
        INIT_ELEMENT_FORMAT(ElementFormat::R32_Float, MTLPixelFormatR32Float, 4);
        INIT_ELEMENT_FORMAT(ElementFormat::R32_UInt, MTLPixelFormatR32Uint, 4);
        INIT_ELEMENT_FORMAT(ElementFormat::R32_SInt, MTLPixelFormatR32Sint, 4);

        INIT_ELEMENT_FORMAT_UNSUPPORT(ElementFormat::RG8_Typeless);
        INIT_ELEMENT_FORMAT(ElementFormat::RG8_UInt, MTLPixelFormatRG8Uint, 2);
        INIT_ELEMENT_FORMAT(ElementFormat::RG8_SInt, MTLPixelFormatRG8Sint, 2);
        INIT_ELEMENT_FORMAT(ElementFormat::RG8_UNorm, MTLPixelFormatRG8Unorm, 2);
        INIT_ELEMENT_FORMAT(ElementFormat::RG8_SNorm, MTLPixelFormatRG8Snorm, 2);

        INIT_ELEMENT_FORMAT_UNSUPPORT(ElementFormat::RG16_Typeless);
        INIT_ELEMENT_FORMAT(ElementFormat::RG16_Float, MTLPixelFormatRG16Float, 4);
        INIT_ELEMENT_FORMAT(ElementFormat::RG16_UInt, MTLPixelFormatRG16Uint, 4);
        INIT_ELEMENT_FORMAT(ElementFormat::RG16_SInt, MTLPixelFormatRG16Sint, 4);
        INIT_ELEMENT_FORMAT(ElementFormat::RG16_UNorm, MTLPixelFormatRG16Unorm, 4);
        INIT_ELEMENT_FORMAT(ElementFormat::RG16_SNorm, MTLPixelFormatRG16Snorm, 4);

        INIT_ELEMENT_FORMAT_UNSUPPORT(ElementFormat::RG32_Typeless);
        INIT_ELEMENT_FORMAT(ElementFormat::RG32_Float, MTLPixelFormatRG32Float, 8);
        INIT_ELEMENT_FORMAT(ElementFormat::RG32_UInt, MTLPixelFormatRG32Uint, 8);
        INIT_ELEMENT_FORMAT(ElementFormat::RG32_SInt, MTLPixelFormatRG32Sint, 8);

        INIT_ELEMENT_FORMAT_UNSUPPORT(ElementFormat::RGB32_Typeless);
        INIT_ELEMENT_FORMAT_UNSUPPORT(ElementFormat::RGB32_Float);
        INIT_ELEMENT_FORMAT_UNSUPPORT(ElementFormat::RGB32_UInt);
        INIT_ELEMENT_FORMAT_UNSUPPORT(ElementFormat::RGB32_SInt);

        INIT_ELEMENT_FORMAT_UNSUPPORT(ElementFormat::RGBA8_Typeless);
        INIT_ELEMENT_FORMAT(ElementFormat::RGBA8_UInt, MTLPixelFormatRGBA8Uint, 4);
        INIT_ELEMENT_FORMAT(ElementFormat::RGBA8_SInt, MTLPixelFormatRGBA8Sint, 4);
        INIT_ELEMENT_FORMAT(ElementFormat::RGBA8_UNorm, MTLPixelFormatRGBA8Unorm, 4);
        INIT_ELEMENT_FORMAT(ElementFormat::RGBA8_SNorm, MTLPixelFormatRGBA8Snorm, 4);
        INIT_ELEMENT_FORMAT(ElementFormat::RGBA8_UNorm_sRGB, MTLPixelFormatRGBA8Unorm_sRGB, 4);

        INIT_ELEMENT_FORMAT_UNSUPPORT(ElementFormat::RGBA16_Typeless);
        INIT_ELEMENT_FORMAT(ElementFormat::RGBA16_Float, MTLPixelFormatRGBA16Float, 8);
        INIT_ELEMENT_FORMAT(ElementFormat::RGBA16_UInt, MTLPixelFormatRGBA16Uint, 8);
        INIT_ELEMENT_FORMAT(ElementFormat::RGBA16_SInt, MTLPixelFormatRGBA16Sint, 8);
        INIT_ELEMENT_FORMAT(ElementFormat::RGBA16_UNorm, MTLPixelFormatRGBA16Unorm, 8);
        INIT_ELEMENT_FORMAT(ElementFormat::RGBA16_SNorm, MTLPixelFormatRGBA16Snorm, 8);

        INIT_ELEMENT_FORMAT_UNSUPPORT(ElementFormat::RGBA32_Typeless);
        INIT_ELEMENT_FORMAT(ElementFormat::RGBA32_Float, MTLPixelFormatRGBA32Float, 16);
        INIT_ELEMENT_FORMAT(ElementFormat::RGBA32_UInt, MTLPixelFormatRGBA32Uint, 16);
        INIT_ELEMENT_FORMAT(ElementFormat::RGBA32_SInt, MTLPixelFormatRGBA32Sint, 16);

        INIT_ELEMENT_FORMAT_UNSUPPORT(ElementFormat::RGB10A2_Typeless);
        INIT_ELEMENT_FORMAT(ElementFormat::RGB10A2_UNorm, MTLPixelFormatRGB10A2Unorm, 4);
        INIT_ELEMENT_FORMAT(ElementFormat::RGB10A2_UInt, MTLPixelFormatRGB10A2Uint, 4);

        INIT_ELEMENT_FORMAT(ElementFormat::RG11B10_Float, MTLPixelFormatRG11B10Float, 4);

        INIT_ELEMENT_FORMAT(ElementFormat::RGB9E5_Exp, MTLPixelFormatRGB9E5Float, 4);

        INIT_ELEMENT_FORMAT(ElementFormat::B5G6R5_UNorm, MTLPixelFormatB5G6R5Unorm, 2);

        INIT_ELEMENT_FORMAT(ElementFormat::BGR5A1_UNorm, MTLPixelFormatBGR5A1Unorm, 2);

        INIT_ELEMENT_FORMAT_UNSUPPORT(ElementFormat::BGRA8_Typeless);
        INIT_ELEMENT_FORMAT(ElementFormat::BGRA8_UNorm, MTLPixelFormatBGRA8Unorm, 4);
        INIT_ELEMENT_FORMAT(ElementFormat::BGRA8_UNorm_sRGB, MTLPixelFormatBGRA8Unorm_sRGB, 4);

        INIT_ELEMENT_FORMAT(ElementFormat::D16_UNorm, MTLPixelFormatDepth16Unorm, 2);
        INIT_ELEMENT_FORMAT(ElementFormat::D24_UNorm_S8_UInt, MTLPixelFormatDepth24Unorm_Stencil8, 4);
        INIT_ELEMENT_FORMAT(ElementFormat::D32_Float, MTLPixelFormatDepth32Float, 4);

        INIT_ELEMENT_FORMAT_UNSUPPORT(ElementFormat::BC1_Typeless);
        INIT_ELEMENT_FORMAT(ElementFormat::BC1_UNorm, MTLPixelFormatBC1_RGBA, 8);
        INIT_ELEMENT_FORMAT(ElementFormat::BC1_UNorm_sRGB, MTLPixelFormatBC1_RGBA_sRGB, 8);

        INIT_ELEMENT_FORMAT_UNSUPPORT(ElementFormat::BC2_Typeless);
        INIT_ELEMENT_FORMAT(ElementFormat::BC2_UNorm, MTLPixelFormatBC2_RGBA, 16);
        INIT_ELEMENT_FORMAT(ElementFormat::BC2_UNorm_sRGB, MTLPixelFormatBC2_RGBA_sRGB, 16);

        INIT_ELEMENT_FORMAT_UNSUPPORT(ElementFormat::BC3_Typeless);
        INIT_ELEMENT_FORMAT(ElementFormat::BC3_UNorm, MTLPixelFormatBC3_RGBA, 16);
        INIT_ELEMENT_FORMAT(ElementFormat::BC3_UNorm_sRGB, MTLPixelFormatBC3_RGBA_sRGB, 16);

        INIT_ELEMENT_FORMAT_UNSUPPORT(ElementFormat::BC4_Typeless);
        INIT_ELEMENT_FORMAT(ElementFormat::BC4_UNorm, MTLPixelFormatBC4_RUnorm, 8);
        INIT_ELEMENT_FORMAT(ElementFormat::BC4_SNorm, MTLPixelFormatBC4_RSnorm, 8);

        INIT_ELEMENT_FORMAT_UNSUPPORT(ElementFormat::BC5_Typeless);
        INIT_ELEMENT_FORMAT(ElementFormat::BC5_UNorm, MTLPixelFormatBC5_RGUnorm, 16);
        INIT_ELEMENT_FORMAT(ElementFormat::BC5_SNorm, MTLPixelFormatBC5_RGSnorm, 16);

        INIT_ELEMENT_FORMAT_UNSUPPORT(ElementFormat::BC6H_Typeless);
        INIT_ELEMENT_FORMAT(ElementFormat::BC6H_UFloat, MTLPixelFormatBC6H_RGBUfloat, 16);
        INIT_ELEMENT_FORMAT(ElementFormat::BC6H_SFloat, MTLPixelFormatBC6H_RGBFloat, 16);

        INIT_ELEMENT_FORMAT_UNSUPPORT(ElementFormat::BC7_Typeless);
        INIT_ELEMENT_FORMAT_UNSUPPORT(ElementFormat::BC7_UNorm);
        INIT_ELEMENT_FORMAT_UNSUPPORT(ElementFormat::BC7_UNorm_sRGB);

#undef INIT_ELEMENT_FORMAT
#undef INIT_ELEMENT_FORMAT_UNSUPPORT

        for (int i = 0; i < (int)ElementFormat::Count; ++i)
        {
            if (gMetalElementFormatInfos[i].unsupported == false && gMetalElementFormatInfos[i].bytes == 0)
            {
                CUBE_LOG(Warning, Metal, "Found uninitialized element format info: {}", i);
            }
        }
    }
} // namespace cube
