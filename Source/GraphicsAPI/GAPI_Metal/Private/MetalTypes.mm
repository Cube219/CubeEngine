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

#define INIT_ELEMENT_FORMAT(elementFormat, inMTLPixelFormat, inMTLVertexFormat, inMTLAttributeFormat, inBytes) \
    { \
        MetalElementFormatInfo& info = gMetalElementFormatInfos[(int)(elementFormat)]; \
        info = { .pixelFormat = (inMTLPixelFormat), .vertexFormat = (inMTLVertexFormat), .attributeFormat = (inMTLAttributeFormat), .bytes = (inBytes) }; \
    }
#define INIT_ELEMENT_FORMAT_UNSUPPORT(elementFormat) INIT_ELEMENT_FORMAT(elementFormat, MTLPixelFormatInvalid, MTLVertexFormatInvalid, MTLAttributeFormatInvalid, 0)

        INIT_ELEMENT_FORMAT_UNSUPPORT(ElementFormat::Unknown);

        INIT_ELEMENT_FORMAT_UNSUPPORT(ElementFormat::R8_Typeless);
        INIT_ELEMENT_FORMAT(ElementFormat::R8_UInt, MTLPixelFormatR8Uint, MTLVertexFormatUChar, MTLAttributeFormatUChar, 1);
        INIT_ELEMENT_FORMAT(ElementFormat::R8_SInt, MTLPixelFormatR8Sint, MTLVertexFormatChar, MTLAttributeFormatChar, 1);
        INIT_ELEMENT_FORMAT(ElementFormat::R8_UNorm, MTLPixelFormatR8Unorm, MTLVertexFormatUCharNormalized, MTLAttributeFormatUCharNormalized, 1);
        INIT_ELEMENT_FORMAT(ElementFormat::R8_SNorm, MTLPixelFormatR8Snorm, MTLVertexFormatCharNormalized, MTLAttributeFormatCharNormalized, 1);

        INIT_ELEMENT_FORMAT_UNSUPPORT(ElementFormat::R16_Typeless);
        INIT_ELEMENT_FORMAT(ElementFormat::R16_Float, MTLPixelFormatR16Float, MTLVertexFormatHalf, MTLAttributeFormatHalf, 2);
        INIT_ELEMENT_FORMAT(ElementFormat::R16_UInt, MTLPixelFormatR16Uint, MTLVertexFormatUShort, MTLAttributeFormatUShort, 2);
        INIT_ELEMENT_FORMAT(ElementFormat::R16_SInt, MTLPixelFormatR16Sint, MTLVertexFormatShort, MTLAttributeFormatShort, 2);
        INIT_ELEMENT_FORMAT(ElementFormat::R16_UNorm, MTLPixelFormatR16Unorm, MTLVertexFormatUShortNormalized, MTLAttributeFormatUShortNormalized, 2);
        INIT_ELEMENT_FORMAT(ElementFormat::R16_SNorm, MTLPixelFormatR16Snorm, MTLVertexFormatShortNormalized, MTLAttributeFormatShortNormalized, 2);

        INIT_ELEMENT_FORMAT_UNSUPPORT(ElementFormat::R32_Typeless);
        INIT_ELEMENT_FORMAT(ElementFormat::R32_Float, MTLPixelFormatR32Float, MTLVertexFormatFloat, MTLAttributeFormatFloat, 4);
        INIT_ELEMENT_FORMAT(ElementFormat::R32_UInt, MTLPixelFormatR32Uint, MTLVertexFormatUInt, MTLAttributeFormatUInt, 4);
        INIT_ELEMENT_FORMAT(ElementFormat::R32_SInt, MTLPixelFormatR32Sint, MTLVertexFormatInt, MTLAttributeFormatInt, 4);

        INIT_ELEMENT_FORMAT_UNSUPPORT(ElementFormat::RG8_Typeless);
        INIT_ELEMENT_FORMAT(ElementFormat::RG8_UInt, MTLPixelFormatRG8Uint, MTLVertexFormatUChar2, MTLAttributeFormatUChar2, 2);
        INIT_ELEMENT_FORMAT(ElementFormat::RG8_SInt, MTLPixelFormatRG8Sint, MTLVertexFormatChar2, MTLAttributeFormatChar2, 2);
        INIT_ELEMENT_FORMAT(ElementFormat::RG8_UNorm, MTLPixelFormatRG8Unorm, MTLVertexFormatUChar2Normalized, MTLAttributeFormatUChar2Normalized, 2);
        INIT_ELEMENT_FORMAT(ElementFormat::RG8_SNorm, MTLPixelFormatRG8Snorm, MTLVertexFormatChar2Normalized, MTLAttributeFormatChar2Normalized, 2);

        INIT_ELEMENT_FORMAT_UNSUPPORT(ElementFormat::RG16_Typeless);
        INIT_ELEMENT_FORMAT(ElementFormat::RG16_Float, MTLPixelFormatRG16Float, MTLVertexFormatHalf2, MTLAttributeFormatHalf2, 4);
        INIT_ELEMENT_FORMAT(ElementFormat::RG16_UInt, MTLPixelFormatRG16Uint, MTLVertexFormatUShort2, MTLAttributeFormatUShort2, 4);
        INIT_ELEMENT_FORMAT(ElementFormat::RG16_SInt, MTLPixelFormatRG16Sint, MTLVertexFormatShort2, MTLAttributeFormatShort2, 4);
        INIT_ELEMENT_FORMAT(ElementFormat::RG16_UNorm, MTLPixelFormatRG16Unorm, MTLVertexFormatUShort2Normalized, MTLAttributeFormatUShort2Normalized, 4);
        INIT_ELEMENT_FORMAT(ElementFormat::RG16_SNorm, MTLPixelFormatRG16Snorm, MTLVertexFormatShort2Normalized, MTLAttributeFormatShort2Normalized, 4);

        INIT_ELEMENT_FORMAT_UNSUPPORT(ElementFormat::RG32_Typeless);
        INIT_ELEMENT_FORMAT(ElementFormat::RG32_Float, MTLPixelFormatRG32Float, MTLVertexFormatFloat2, MTLAttributeFormatFloat2, 8);
        INIT_ELEMENT_FORMAT(ElementFormat::RG32_UInt, MTLPixelFormatRG32Uint, MTLVertexFormatUInt2, MTLAttributeFormatUInt2, 8);
        INIT_ELEMENT_FORMAT(ElementFormat::RG32_SInt, MTLPixelFormatRG32Sint, MTLVertexFormatInt2, MTLAttributeFormatInt2, 8);

        INIT_ELEMENT_FORMAT_UNSUPPORT(ElementFormat::RGB32_Typeless);
        INIT_ELEMENT_FORMAT(ElementFormat::RGB32_Float, MTLPixelFormatInvalid, MTLVertexFormatFloat3, MTLAttributeFormatFloat3, 12);
        INIT_ELEMENT_FORMAT(ElementFormat::RGB32_UInt, MTLPixelFormatInvalid, MTLVertexFormatUInt3, MTLAttributeFormatUInt3, 12);
        INIT_ELEMENT_FORMAT(ElementFormat::RGB32_SInt, MTLPixelFormatInvalid, MTLVertexFormatInt3, MTLAttributeFormatInt3, 12);

        INIT_ELEMENT_FORMAT_UNSUPPORT(ElementFormat::RGBA8_Typeless);
        INIT_ELEMENT_FORMAT(ElementFormat::RGBA8_UInt, MTLPixelFormatRGBA8Uint, MTLVertexFormatUChar4, MTLAttributeFormatUChar4, 4);
        INIT_ELEMENT_FORMAT(ElementFormat::RGBA8_SInt, MTLPixelFormatRGBA8Sint, MTLVertexFormatChar4, MTLAttributeFormatChar4, 4);
        INIT_ELEMENT_FORMAT(ElementFormat::RGBA8_UNorm, MTLPixelFormatRGBA8Unorm, MTLVertexFormatUChar4Normalized, MTLAttributeFormatUChar4Normalized, 4);
        INIT_ELEMENT_FORMAT(ElementFormat::RGBA8_SNorm, MTLPixelFormatRGBA8Snorm, MTLVertexFormatChar4Normalized, MTLAttributeFormatChar4Normalized, 4);
        INIT_ELEMENT_FORMAT(ElementFormat::RGBA8_UNorm_sRGB, MTLPixelFormatRGBA8Unorm_sRGB, MTLVertexFormatInvalid, MTLAttributeFormatInvalid, 4);

        INIT_ELEMENT_FORMAT_UNSUPPORT(ElementFormat::RGBA16_Typeless);
        INIT_ELEMENT_FORMAT(ElementFormat::RGBA16_Float, MTLPixelFormatRGBA16Float, MTLVertexFormatHalf4, MTLAttributeFormatHalf4, 8);
        INIT_ELEMENT_FORMAT(ElementFormat::RGBA16_UInt, MTLPixelFormatRGBA16Uint, MTLVertexFormatUShort4, MTLAttributeFormatUShort4, 8);
        INIT_ELEMENT_FORMAT(ElementFormat::RGBA16_SInt, MTLPixelFormatRGBA16Sint, MTLVertexFormatShort4, MTLAttributeFormatShort4, 8);
        INIT_ELEMENT_FORMAT(ElementFormat::RGBA16_UNorm, MTLPixelFormatRGBA16Unorm, MTLVertexFormatUShort4Normalized, MTLAttributeFormatUShort4Normalized, 8);
        INIT_ELEMENT_FORMAT(ElementFormat::RGBA16_SNorm, MTLPixelFormatRGBA16Snorm, MTLVertexFormatShort4Normalized, MTLAttributeFormatShort4Normalized, 8);

        INIT_ELEMENT_FORMAT_UNSUPPORT(ElementFormat::RGBA32_Typeless);
        INIT_ELEMENT_FORMAT(ElementFormat::RGBA32_Float, MTLPixelFormatRGBA32Float, MTLVertexFormatFloat4, MTLAttributeFormatFloat4, 16);
        INIT_ELEMENT_FORMAT(ElementFormat::RGBA32_UInt, MTLPixelFormatRGBA32Uint, MTLVertexFormatUInt4, MTLAttributeFormatUInt4, 16);
        INIT_ELEMENT_FORMAT(ElementFormat::RGBA32_SInt, MTLPixelFormatRGBA32Sint, MTLVertexFormatInt4, MTLAttributeFormatInt4, 16);

        INIT_ELEMENT_FORMAT_UNSUPPORT(ElementFormat::RGB10A2_Typeless);
        INIT_ELEMENT_FORMAT(ElementFormat::RGB10A2_UNorm, MTLPixelFormatRGB10A2Unorm, MTLVertexFormatUInt1010102Normalized, MTLAttributeFormatUInt1010102Normalized, 4);
        INIT_ELEMENT_FORMAT(ElementFormat::RGB10A2_UInt, MTLPixelFormatRGB10A2Uint, MTLVertexFormatInvalid, MTLAttributeFormatInvalid, 4);

        INIT_ELEMENT_FORMAT(ElementFormat::RG11B10_Float, MTLPixelFormatRG11B10Float, MTLVertexFormatFloatRG11B10, MTLAttributeFormatFloatRG11B10, 4);

        INIT_ELEMENT_FORMAT(ElementFormat::RGB9E5_Exp, MTLPixelFormatRGB9E5Float, MTLVertexFormatFloatRGB9E5, MTLAttributeFormatFloatRGB9E5, 4);

        INIT_ELEMENT_FORMAT(ElementFormat::B5G6R5_UNorm, MTLPixelFormatB5G6R5Unorm, MTLVertexFormatInvalid, MTLAttributeFormatInvalid, 2);

        INIT_ELEMENT_FORMAT(ElementFormat::BGR5A1_UNorm, MTLPixelFormatBGR5A1Unorm, MTLVertexFormatInvalid, MTLAttributeFormatInvalid, 2);

        INIT_ELEMENT_FORMAT_UNSUPPORT(ElementFormat::BGRA8_Typeless);
        INIT_ELEMENT_FORMAT(ElementFormat::BGRA8_UNorm, MTLPixelFormatBGRA8Unorm, MTLVertexFormatUChar4Normalized_BGRA, MTLAttributeFormatUChar4Normalized_BGRA, 4);
        INIT_ELEMENT_FORMAT(ElementFormat::BGRA8_UNorm_sRGB, MTLPixelFormatBGRA8Unorm_sRGB, MTLVertexFormatInvalid, MTLAttributeFormatInvalid, 4);

        INIT_ELEMENT_FORMAT(ElementFormat::D16_UNorm, MTLPixelFormatDepth16Unorm, MTLVertexFormatInvalid, MTLAttributeFormatInvalid, 2);
        INIT_ELEMENT_FORMAT(ElementFormat::D24_UNorm_S8_UInt, MTLPixelFormatDepth24Unorm_Stencil8, MTLVertexFormatInvalid, MTLAttributeFormatInvalid, 4);
        INIT_ELEMENT_FORMAT(ElementFormat::D32_Float, MTLPixelFormatDepth32Float, MTLVertexFormatInvalid, MTLAttributeFormatInvalid, 4);

        INIT_ELEMENT_FORMAT_UNSUPPORT(ElementFormat::BC1_Typeless);
        INIT_ELEMENT_FORMAT(ElementFormat::BC1_UNorm, MTLPixelFormatBC1_RGBA, MTLVertexFormatInvalid, MTLAttributeFormatInvalid, 8);
        INIT_ELEMENT_FORMAT(ElementFormat::BC1_UNorm_sRGB, MTLPixelFormatBC1_RGBA_sRGB, MTLVertexFormatInvalid, MTLAttributeFormatInvalid, 8);

        INIT_ELEMENT_FORMAT_UNSUPPORT(ElementFormat::BC2_Typeless);
        INIT_ELEMENT_FORMAT(ElementFormat::BC2_UNorm, MTLPixelFormatBC2_RGBA, MTLVertexFormatInvalid, MTLAttributeFormatInvalid, 16);
        INIT_ELEMENT_FORMAT(ElementFormat::BC2_UNorm_sRGB, MTLPixelFormatBC2_RGBA_sRGB, MTLVertexFormatInvalid, MTLAttributeFormatInvalid, 16);

        INIT_ELEMENT_FORMAT_UNSUPPORT(ElementFormat::BC3_Typeless);
        INIT_ELEMENT_FORMAT(ElementFormat::BC3_UNorm, MTLPixelFormatBC3_RGBA, MTLVertexFormatInvalid, MTLAttributeFormatInvalid, 16);
        INIT_ELEMENT_FORMAT(ElementFormat::BC3_UNorm_sRGB, MTLPixelFormatBC3_RGBA_sRGB, MTLVertexFormatInvalid, MTLAttributeFormatInvalid, 16);

        INIT_ELEMENT_FORMAT_UNSUPPORT(ElementFormat::BC4_Typeless);
        INIT_ELEMENT_FORMAT(ElementFormat::BC4_UNorm, MTLPixelFormatBC4_RUnorm, MTLVertexFormatInvalid, MTLAttributeFormatInvalid, 8);
        INIT_ELEMENT_FORMAT(ElementFormat::BC4_SNorm, MTLPixelFormatBC4_RSnorm, MTLVertexFormatInvalid, MTLAttributeFormatInvalid, 8);

        INIT_ELEMENT_FORMAT_UNSUPPORT(ElementFormat::BC5_Typeless);
        INIT_ELEMENT_FORMAT(ElementFormat::BC5_UNorm, MTLPixelFormatBC5_RGUnorm, MTLVertexFormatInvalid, MTLAttributeFormatInvalid, 16);
        INIT_ELEMENT_FORMAT(ElementFormat::BC5_SNorm, MTLPixelFormatBC5_RGSnorm, MTLVertexFormatInvalid, MTLAttributeFormatInvalid, 16);

        INIT_ELEMENT_FORMAT_UNSUPPORT(ElementFormat::BC6H_Typeless);
        INIT_ELEMENT_FORMAT(ElementFormat::BC6H_UFloat, MTLPixelFormatBC6H_RGBUfloat, MTLVertexFormatInvalid, MTLAttributeFormatInvalid, 16);
        INIT_ELEMENT_FORMAT(ElementFormat::BC6H_SFloat, MTLPixelFormatBC6H_RGBFloat, MTLVertexFormatInvalid, MTLAttributeFormatInvalid, 16);

        INIT_ELEMENT_FORMAT_UNSUPPORT(ElementFormat::BC7_Typeless);
        INIT_ELEMENT_FORMAT_UNSUPPORT(ElementFormat::BC7_UNorm);
        INIT_ELEMENT_FORMAT_UNSUPPORT(ElementFormat::BC7_UNorm_sRGB);

#undef INIT_ELEMENT_FORMAT
#undef INIT_ELEMENT_FORMAT_UNSUPPORT

#if CUBE_USE_CHECK
        for (int i = 0; i < (int)ElementFormat::Count; ++i)
        {
            const bool hasAnyNonInvalidFormat = (gMetalElementFormatInfos[i].pixelFormat != MTLPixelFormatInvalid)
                || (gMetalElementFormatInfos[i].vertexFormat != MTLVertexFormatInvalid)
                || (gMetalElementFormatInfos[i].attributeFormat != MTLAttributeFormatInvalid);
            if (hasAnyNonInvalidFormat && gMetalElementFormatInfos[i].bytes == 0)
            {
                CUBE_LOG(Warning, Metal, "Found uninitialized element format info: {0}", i);
            }
        }
#endif // CUBE_USE_CHECK
    }
} // namespace cube
