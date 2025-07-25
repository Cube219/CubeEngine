#include "DX12Types.h"

#include "Logger.h"

namespace cube
{
    DX12ElementFormatInfo gDX12ElementFormatInfos[(int)gapi::ElementFormat::Count] = {};

    DX12ElementFormatInfo GetDX12ElementFormatInfo(gapi::ElementFormat elementFormat)
    {
        return gDX12ElementFormatInfos[(int)elementFormat];
    }

    void InitializeTypes()
    {
        using namespace gapi;

#define INIT_ELEMENT_FORMAT(elementFormat, dxgiFormat, inBytes) \
        gDX12ElementFormatInfos[(int)(elementFormat)] = { .format = (dxgiFormat), .bytes = (inBytes) }
#define INIT_ELEMENT_FORMAT_UNSUPPORT(elementFormat) \
        gDX12ElementFormatInfos[(int)(elementFormat)].unsupported = true

        INIT_ELEMENT_FORMAT_UNSUPPORT(ElementFormat::Unknown);

        INIT_ELEMENT_FORMAT(ElementFormat::R8_Typeless, DXGI_FORMAT_R8_TYPELESS, 1);
        INIT_ELEMENT_FORMAT(ElementFormat::R8_UInt, DXGI_FORMAT_R8_UINT, 1);
        INIT_ELEMENT_FORMAT(ElementFormat::R8_SInt, DXGI_FORMAT_R8_SINT, 1);
        INIT_ELEMENT_FORMAT(ElementFormat::R8_UNorm, DXGI_FORMAT_R8_UNORM, 1);
        INIT_ELEMENT_FORMAT(ElementFormat::R8_SNorm, DXGI_FORMAT_R8_SNORM, 1);

        INIT_ELEMENT_FORMAT(ElementFormat::R16_Typeless, DXGI_FORMAT_R16_TYPELESS, 2);
        INIT_ELEMENT_FORMAT(ElementFormat::R16_Float, DXGI_FORMAT_R16_FLOAT, 2);
        INIT_ELEMENT_FORMAT(ElementFormat::R16_UInt, DXGI_FORMAT_R16_UINT, 2);
        INIT_ELEMENT_FORMAT(ElementFormat::R16_SInt, DXGI_FORMAT_R16_SINT, 2);
        INIT_ELEMENT_FORMAT(ElementFormat::R16_UNorm, DXGI_FORMAT_R16_UNORM, 2);
        INIT_ELEMENT_FORMAT(ElementFormat::R16_SNorm, DXGI_FORMAT_R16_SNORM, 2);

        INIT_ELEMENT_FORMAT(ElementFormat::R32_Typeless, DXGI_FORMAT_R32_TYPELESS, 4);
        INIT_ELEMENT_FORMAT(ElementFormat::R32_Float, DXGI_FORMAT_R32_FLOAT, 4);
        INIT_ELEMENT_FORMAT(ElementFormat::R32_UInt, DXGI_FORMAT_R32_UINT, 4);
        INIT_ELEMENT_FORMAT(ElementFormat::R32_SInt, DXGI_FORMAT_R32_SINT, 4);

        INIT_ELEMENT_FORMAT(ElementFormat::RG8_Typeless, DXGI_FORMAT_R8G8_TYPELESS, 2);
        INIT_ELEMENT_FORMAT(ElementFormat::RG8_UInt, DXGI_FORMAT_R8G8_UINT, 2);
        INIT_ELEMENT_FORMAT(ElementFormat::RG8_SInt, DXGI_FORMAT_R8G8_SINT, 2);
        INIT_ELEMENT_FORMAT(ElementFormat::RG8_UNorm, DXGI_FORMAT_R8G8_UNORM, 2);
        INIT_ELEMENT_FORMAT(ElementFormat::RG8_SNorm, DXGI_FORMAT_R8G8_SNORM, 2);

        INIT_ELEMENT_FORMAT(ElementFormat::RG16_Typeless, DXGI_FORMAT_R16G16_TYPELESS, 4);
        INIT_ELEMENT_FORMAT(ElementFormat::RG16_Float, DXGI_FORMAT_R16G16_FLOAT, 4);
        INIT_ELEMENT_FORMAT(ElementFormat::RG16_UInt, DXGI_FORMAT_R16G16_UINT, 4);
        INIT_ELEMENT_FORMAT(ElementFormat::RG16_SInt, DXGI_FORMAT_R16G16_SINT, 4);
        INIT_ELEMENT_FORMAT(ElementFormat::RG16_UNorm, DXGI_FORMAT_R16G16_UNORM, 4);
        INIT_ELEMENT_FORMAT(ElementFormat::RG16_SNorm, DXGI_FORMAT_R16G16_SNORM, 4);

        INIT_ELEMENT_FORMAT(ElementFormat::RG32_Typeless, DXGI_FORMAT_R32G32_TYPELESS, 8);
        INIT_ELEMENT_FORMAT(ElementFormat::RG32_Float, DXGI_FORMAT_R32G32_FLOAT, 8);
        INIT_ELEMENT_FORMAT(ElementFormat::RG32_UInt, DXGI_FORMAT_R32G32_UINT, 8);
        INIT_ELEMENT_FORMAT(ElementFormat::RG32_SInt, DXGI_FORMAT_R32G32_SINT, 8);

        INIT_ELEMENT_FORMAT(ElementFormat::RGB32_Typeless, DXGI_FORMAT_R32G32B32_TYPELESS, 12);
        INIT_ELEMENT_FORMAT(ElementFormat::RGB32_Float, DXGI_FORMAT_R32G32B32_FLOAT, 12);
        INIT_ELEMENT_FORMAT(ElementFormat::RGB32_UInt, DXGI_FORMAT_R32G32B32_UINT, 12);
        INIT_ELEMENT_FORMAT(ElementFormat::RGB32_SInt, DXGI_FORMAT_R32G32B32_SINT, 12);

        INIT_ELEMENT_FORMAT(ElementFormat::RGBA8_Typeless, DXGI_FORMAT_R8G8B8A8_TYPELESS, 4);
        INIT_ELEMENT_FORMAT(ElementFormat::RGBA8_UInt, DXGI_FORMAT_R8G8B8A8_UINT, 4);
        INIT_ELEMENT_FORMAT(ElementFormat::RGBA8_SInt, DXGI_FORMAT_R8G8B8A8_SINT, 4);
        INIT_ELEMENT_FORMAT(ElementFormat::RGBA8_UNorm, DXGI_FORMAT_R8G8B8A8_UNORM, 4);
        INIT_ELEMENT_FORMAT(ElementFormat::RGBA8_SNorm, DXGI_FORMAT_R8G8B8A8_SNORM, 4);
        INIT_ELEMENT_FORMAT(ElementFormat::RGBA8_UNorm_sRGB, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, 4);

        INIT_ELEMENT_FORMAT(ElementFormat::RGBA16_Typeless, DXGI_FORMAT_R16G16B16A16_TYPELESS, 8);
        INIT_ELEMENT_FORMAT(ElementFormat::RGBA16_Float, DXGI_FORMAT_R16G16B16A16_FLOAT, 8);
        INIT_ELEMENT_FORMAT(ElementFormat::RGBA16_UInt, DXGI_FORMAT_R16G16B16A16_UINT, 8);
        INIT_ELEMENT_FORMAT(ElementFormat::RGBA16_SInt, DXGI_FORMAT_R16G16B16A16_SINT, 8);
        INIT_ELEMENT_FORMAT(ElementFormat::RGBA16_UNorm, DXGI_FORMAT_R16G16B16A16_UNORM, 8);
        INIT_ELEMENT_FORMAT(ElementFormat::RGBA16_SNorm, DXGI_FORMAT_R16G16B16A16_SNORM, 8);

        INIT_ELEMENT_FORMAT(ElementFormat::RGBA32_Typeless, DXGI_FORMAT_R32G32B32A32_TYPELESS, 16);
        INIT_ELEMENT_FORMAT(ElementFormat::RGBA32_Float, DXGI_FORMAT_R32G32B32A32_FLOAT, 16);
        INIT_ELEMENT_FORMAT(ElementFormat::RGBA32_UInt, DXGI_FORMAT_R32G32B32A32_UINT, 16);
        INIT_ELEMENT_FORMAT(ElementFormat::RGBA32_SInt, DXGI_FORMAT_R32G32B32A32_SINT, 16);

        INIT_ELEMENT_FORMAT(ElementFormat::RGB10A2_Typeless, DXGI_FORMAT_R10G10B10A2_TYPELESS, 4);
        INIT_ELEMENT_FORMAT(ElementFormat::RGB10A2_UNorm, DXGI_FORMAT_R10G10B10A2_UNORM, 4);
        INIT_ELEMENT_FORMAT(ElementFormat::RGB10A2_UInt, DXGI_FORMAT_R10G10B10A2_UINT, 4);

        INIT_ELEMENT_FORMAT(ElementFormat::RG11B10_Float, DXGI_FORMAT_R11G11B10_FLOAT, 4);

        INIT_ELEMENT_FORMAT(ElementFormat::RGB9E5_Exp, DXGI_FORMAT_R9G9B9E5_SHAREDEXP, 4);

        INIT_ELEMENT_FORMAT(ElementFormat::B5G6R5_UNorm, DXGI_FORMAT_B5G6R5_UNORM, 2);

        INIT_ELEMENT_FORMAT(ElementFormat::BGR5A1_UNorm, DXGI_FORMAT_B5G5R5A1_UNORM, 2);

        INIT_ELEMENT_FORMAT(ElementFormat::BGRA8_Typeless, DXGI_FORMAT_B8G8R8A8_TYPELESS, 4);
        INIT_ELEMENT_FORMAT(ElementFormat::BGRA8_UNorm, DXGI_FORMAT_B8G8R8A8_UNORM, 4);
        INIT_ELEMENT_FORMAT(ElementFormat::BGRA8_UNorm_sRGB, DXGI_FORMAT_B8G8R8A8_UNORM_SRGB, 4);

        INIT_ELEMENT_FORMAT(ElementFormat::D16_UNorm, DXGI_FORMAT_D16_UNORM, 2);
        INIT_ELEMENT_FORMAT(ElementFormat::D24_UNorm_S8_UInt, DXGI_FORMAT_D24_UNORM_S8_UINT, 4);
        INIT_ELEMENT_FORMAT(ElementFormat::D32_Float, DXGI_FORMAT_D32_FLOAT, 4);

        INIT_ELEMENT_FORMAT(ElementFormat::BC1_Typeless, DXGI_FORMAT_BC1_TYPELESS, 8);
        INIT_ELEMENT_FORMAT(ElementFormat::BC1_UNorm, DXGI_FORMAT_BC1_UNORM, 8);
        INIT_ELEMENT_FORMAT(ElementFormat::BC1_UNorm_sRGB, DXGI_FORMAT_BC1_UNORM_SRGB, 8);

        INIT_ELEMENT_FORMAT(ElementFormat::BC2_Typeless, DXGI_FORMAT_BC2_TYPELESS, 16);
        INIT_ELEMENT_FORMAT(ElementFormat::BC2_UNorm, DXGI_FORMAT_BC2_UNORM, 16);
        INIT_ELEMENT_FORMAT(ElementFormat::BC2_UNorm_sRGB, DXGI_FORMAT_BC2_UNORM_SRGB, 16);

        INIT_ELEMENT_FORMAT(ElementFormat::BC3_Typeless, DXGI_FORMAT_BC3_TYPELESS, 16);
        INIT_ELEMENT_FORMAT(ElementFormat::BC3_UNorm, DXGI_FORMAT_BC3_UNORM, 16);
        INIT_ELEMENT_FORMAT(ElementFormat::BC3_UNorm_sRGB, DXGI_FORMAT_BC3_UNORM_SRGB, 16);

        INIT_ELEMENT_FORMAT(ElementFormat::BC4_Typeless, DXGI_FORMAT_BC4_TYPELESS, 8);
        INIT_ELEMENT_FORMAT(ElementFormat::BC4_UNorm, DXGI_FORMAT_BC4_UNORM, 8);
        INIT_ELEMENT_FORMAT(ElementFormat::BC4_SNorm, DXGI_FORMAT_BC4_SNORM, 8);

        INIT_ELEMENT_FORMAT(ElementFormat::BC5_Typeless, DXGI_FORMAT_BC5_TYPELESS, 16);
        INIT_ELEMENT_FORMAT(ElementFormat::BC5_UNorm, DXGI_FORMAT_BC5_UNORM, 16);
        INIT_ELEMENT_FORMAT(ElementFormat::BC5_SNorm, DXGI_FORMAT_BC5_SNORM, 16);

        INIT_ELEMENT_FORMAT(ElementFormat::BC6H_Typeless, DXGI_FORMAT_BC6H_TYPELESS, 16);
        INIT_ELEMENT_FORMAT(ElementFormat::BC6H_UFloat, DXGI_FORMAT_BC6H_UF16, 16);
        INIT_ELEMENT_FORMAT(ElementFormat::BC6H_SFloat, DXGI_FORMAT_BC6H_SF16, 16);

        INIT_ELEMENT_FORMAT(ElementFormat::BC7_Typeless, DXGI_FORMAT_BC7_TYPELESS, 16);
        INIT_ELEMENT_FORMAT(ElementFormat::BC7_UNorm, DXGI_FORMAT_BC7_UNORM, 16);
        INIT_ELEMENT_FORMAT(ElementFormat::BC7_UNorm_sRGB, DXGI_FORMAT_BC7_UNORM_SRGB, 16);

#undef INIT_ELEMENT_FORMAT_UNSUPPORT
#undef INIT_ELEMENT_FORMAT

        for (int i = 0; i < (int)ElementFormat::Count; ++i)
        {
            if (gDX12ElementFormatInfos[i].unsupported == false && gDX12ElementFormatInfos[i].bytes == 0)
            {
                CUBE_LOG(Warning, DX12, "Found uninitialized element format info: {}", i);
            }
        }
    }
} // namespace cube
