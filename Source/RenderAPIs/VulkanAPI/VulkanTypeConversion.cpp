#include "VulkanTypeConversion.h"

namespace cube
{
    namespace rapi
    {
        void InitTypeConversion()
        {
            texFmtToVkFmt[(Uint32)TextureFormat::Unknown] = VK_FORMAT_UNDEFINED;

            texFmtToVkFmt[(Uint32)TextureFormat::R_8_Uint] = VK_FORMAT_R8_UINT;
            texFmtToVkFmt[(Uint32)TextureFormat::R_8_Sint] = VK_FORMAT_R8_SINT;
            texFmtToVkFmt[(Uint32)TextureFormat::R_8_UNorm] = VK_FORMAT_R8_UNORM;
            texFmtToVkFmt[(Uint32)TextureFormat::R_8_SNorm] = VK_FORMAT_R8_SNORM;
            texFmtToVkFmt[(Uint32)TextureFormat::R_8_sRGB] = VK_FORMAT_R8_SRGB;

            texFmtToVkFmt[(Uint32)TextureFormat::R_16_Float] = VK_FORMAT_R16_SFLOAT;
            texFmtToVkFmt[(Uint32)TextureFormat::R_16_Uint] = VK_FORMAT_R16_UINT;
            texFmtToVkFmt[(Uint32)TextureFormat::R_16_Sint] = VK_FORMAT_R16_SINT;
            texFmtToVkFmt[(Uint32)TextureFormat::R_16_UNorm] = VK_FORMAT_R16_UNORM;
            texFmtToVkFmt[(Uint32)TextureFormat::R_16_SNorm] = VK_FORMAT_R16_SNORM;

            texFmtToVkFmt[(Uint32)TextureFormat::R_32_Float] = VK_FORMAT_R32_SFLOAT;
            texFmtToVkFmt[(Uint32)TextureFormat::R_32_Uint] = VK_FORMAT_R32_UINT;
            texFmtToVkFmt[(Uint32)TextureFormat::R_32_Sint] = VK_FORMAT_R32_SINT;

            texFmtToVkFmt[(Uint32)TextureFormat::R_64_Float] = VK_FORMAT_R64_SFLOAT;
            texFmtToVkFmt[(Uint32)TextureFormat::R_64_Uint] = VK_FORMAT_R64_UINT;
            texFmtToVkFmt[(Uint32)TextureFormat::R_64_Sint] = VK_FORMAT_R64_SINT;

            texFmtToVkFmt[(Uint32)TextureFormat::RG_8_Uint] = VK_FORMAT_R8G8_UINT;
            texFmtToVkFmt[(Uint32)TextureFormat::RG_8_Sint] = VK_FORMAT_R8G8_SINT;
            texFmtToVkFmt[(Uint32)TextureFormat::RG_8_UNorm] = VK_FORMAT_R8G8_UNORM;
            texFmtToVkFmt[(Uint32)TextureFormat::RG_8_SNorm] = VK_FORMAT_R8G8_SNORM;
            texFmtToVkFmt[(Uint32)TextureFormat::RG_8_sRGB] = VK_FORMAT_R8G8_SRGB;

            texFmtToVkFmt[(Uint32)TextureFormat::RG_16_Float] = VK_FORMAT_R16G16_SFLOAT;
            texFmtToVkFmt[(Uint32)TextureFormat::RG_16_Uint] = VK_FORMAT_R16G16_UINT;
            texFmtToVkFmt[(Uint32)TextureFormat::RG_16_Sint] = VK_FORMAT_R16G16_SINT;
            texFmtToVkFmt[(Uint32)TextureFormat::RG_16_UNorm] = VK_FORMAT_R16G16_UNORM;
            texFmtToVkFmt[(Uint32)TextureFormat::RG_16_SNorm] = VK_FORMAT_R16G16_SNORM;

            texFmtToVkFmt[(Uint32)TextureFormat::RG_32_Float] = VK_FORMAT_R32G32_SFLOAT;
            texFmtToVkFmt[(Uint32)TextureFormat::RG_32_Uint] = VK_FORMAT_R32G32_UINT;
            texFmtToVkFmt[(Uint32)TextureFormat::RG_32_Sint] = VK_FORMAT_R32G32_SINT;

            texFmtToVkFmt[(Uint32)TextureFormat::RG_64_Float] = VK_FORMAT_R64G64_SFLOAT;
            texFmtToVkFmt[(Uint32)TextureFormat::RG_64_Uint] = VK_FORMAT_R64G64_UINT;
            texFmtToVkFmt[(Uint32)TextureFormat::RG_64_Sint] = VK_FORMAT_R64G64_SINT;

            texFmtToVkFmt[(Uint32)TextureFormat::RGB_8_Uint] = VK_FORMAT_R8G8B8_UINT;
            texFmtToVkFmt[(Uint32)TextureFormat::RGB_8_Sint] = VK_FORMAT_R8G8B8_SINT;
            texFmtToVkFmt[(Uint32)TextureFormat::RGB_8_UNorm] = VK_FORMAT_R8G8B8_UNORM;
            texFmtToVkFmt[(Uint32)TextureFormat::RGB_8_SNorm] = VK_FORMAT_R8G8B8_SNORM;
            texFmtToVkFmt[(Uint32)TextureFormat::RGB_8_sRGB] = VK_FORMAT_R8G8B8_SRGB;

            texFmtToVkFmt[(Uint32)TextureFormat::RGB_16_Float] = VK_FORMAT_R16G16B16_SFLOAT;
            texFmtToVkFmt[(Uint32)TextureFormat::RGB_16_Uint] = VK_FORMAT_R16G16B16_UINT;
            texFmtToVkFmt[(Uint32)TextureFormat::RGB_16_Sint] = VK_FORMAT_R16G16B16_SINT;
            texFmtToVkFmt[(Uint32)TextureFormat::RGB_16_UNorm] = VK_FORMAT_R16G16B16_UNORM;
            texFmtToVkFmt[(Uint32)TextureFormat::RGB_16_SNorm] = VK_FORMAT_R16G16B16_SNORM;

            texFmtToVkFmt[(Uint32)TextureFormat::RGB_32_Float] = VK_FORMAT_R32G32B32_SFLOAT;
            texFmtToVkFmt[(Uint32)TextureFormat::RGB_32_Uint] = VK_FORMAT_R32G32B32_UINT;
            texFmtToVkFmt[(Uint32)TextureFormat::RGB_32_Sint] = VK_FORMAT_R32G32B32_SINT;

            texFmtToVkFmt[(Uint32)TextureFormat::RGB_64_Float] = VK_FORMAT_R64G64B64_SFLOAT;
            texFmtToVkFmt[(Uint32)TextureFormat::RGB_64_Uint] = VK_FORMAT_R64G64B64_UINT;
            texFmtToVkFmt[(Uint32)TextureFormat::RGB_64_Sint] = VK_FORMAT_R64G64B64_SINT;

            texFmtToVkFmt[(Uint32)TextureFormat::BGR_8_Uint] = VK_FORMAT_B8G8R8_UINT;
            texFmtToVkFmt[(Uint32)TextureFormat::BGR_8_Sint] = VK_FORMAT_B8G8R8_SINT;
            texFmtToVkFmt[(Uint32)TextureFormat::BGR_8_UNorm] = VK_FORMAT_B8G8R8_UNORM;
            texFmtToVkFmt[(Uint32)TextureFormat::BGR_8_SNorm] = VK_FORMAT_B8G8R8_SNORM;
            texFmtToVkFmt[(Uint32)TextureFormat::BGR_8_sRGB] = VK_FORMAT_B8G8R8_SRGB;

            texFmtToVkFmt[(Uint32)TextureFormat::RGBA_8_Uint] = VK_FORMAT_R8G8B8A8_UINT;
            texFmtToVkFmt[(Uint32)TextureFormat::RGBA_8_Sint] = VK_FORMAT_R8G8B8A8_SINT;
            texFmtToVkFmt[(Uint32)TextureFormat::RGBA_8_UNorm] = VK_FORMAT_R8G8B8A8_UNORM;
            texFmtToVkFmt[(Uint32)TextureFormat::RGBA_8_SNorm] = VK_FORMAT_R8G8B8A8_SNORM;
            texFmtToVkFmt[(Uint32)TextureFormat::RGBA_8_sRGB] = VK_FORMAT_R8G8B8A8_SRGB;

            texFmtToVkFmt[(Uint32)TextureFormat::RGBA_16_Float] = VK_FORMAT_R16G16B16A16_SFLOAT;
            texFmtToVkFmt[(Uint32)TextureFormat::RGBA_16_Uint] = VK_FORMAT_R16G16B16A16_UINT;
            texFmtToVkFmt[(Uint32)TextureFormat::RGBA_16_Sint] = VK_FORMAT_R16G16B16A16_SINT;
            texFmtToVkFmt[(Uint32)TextureFormat::RGBA_16_UNorm] = VK_FORMAT_R16G16B16A16_UNORM;
            texFmtToVkFmt[(Uint32)TextureFormat::RGBA_16_SNorm] = VK_FORMAT_R16G16B16A16_SNORM;

            texFmtToVkFmt[(Uint32)TextureFormat::RGBA_32_Float] = VK_FORMAT_R32G32B32A32_SFLOAT;
            texFmtToVkFmt[(Uint32)TextureFormat::RGBA_32_Uint] = VK_FORMAT_R32G32B32A32_UINT;
            texFmtToVkFmt[(Uint32)TextureFormat::RGBA_32_Sint] = VK_FORMAT_R32G32B32A32_SINT;

            texFmtToVkFmt[(Uint32)TextureFormat::RGBA_64_Float] = VK_FORMAT_R64G64B64A64_SFLOAT;
            texFmtToVkFmt[(Uint32)TextureFormat::RGBA_64_Uint] = VK_FORMAT_R64G64B64A64_UINT;
            texFmtToVkFmt[(Uint32)TextureFormat::RGBA_64_Sint] = VK_FORMAT_R64G64B64A64_SINT;

            texFmtToVkFmt[(Uint32)TextureFormat::BGRA_8_Uint] = VK_FORMAT_B8G8R8A8_UINT;
            texFmtToVkFmt[(Uint32)TextureFormat::BGRA_8_Sint] = VK_FORMAT_B8G8R8A8_SINT;
            texFmtToVkFmt[(Uint32)TextureFormat::BGRA_8_UNorm] = VK_FORMAT_B8G8R8A8_UNORM;
            texFmtToVkFmt[(Uint32)TextureFormat::BGRA_8_SNorm] = VK_FORMAT_B8G8R8A8_SNORM;
            texFmtToVkFmt[(Uint32)TextureFormat::BGRA_8_sRGB] = VK_FORMAT_B8G8R8A8_SRGB;

            // ABGR_8 // 써도 되나 확인

            // A2RGB_10 // 써도 되나 확인

            // A2BGR_10 // 써도 되나 확인

            texFmtToVkFmt[(Uint32)TextureFormat::D16_UNorm] = VK_FORMAT_D16_UNORM;

            texFmtToVkFmt[(Uint32)TextureFormat::D32_Float] = VK_FORMAT_D32_SFLOAT;

            texFmtToVkFmt[(Uint32)TextureFormat::BC1_RGB_UNorm] = VK_FORMAT_BC1_RGB_UNORM_BLOCK;
            texFmtToVkFmt[(Uint32)TextureFormat::BC1_RGB_sRGB] = VK_FORMAT_BC1_RGB_SRGB_BLOCK;
            texFmtToVkFmt[(Uint32)TextureFormat::BC1_RGBA_UNorm] = VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
            texFmtToVkFmt[(Uint32)TextureFormat::BC1_RGBA_sRGB] = VK_FORMAT_BC1_RGBA_SRGB_BLOCK;

            texFmtToVkFmt[(Uint32)TextureFormat::BC2_UNorm] = VK_FORMAT_BC2_UNORM_BLOCK;
            texFmtToVkFmt[(Uint32)TextureFormat::BC2_sRGB] = VK_FORMAT_BC2_SRGB_BLOCK;

            texFmtToVkFmt[(Uint32)TextureFormat::BC3_UNorm] = VK_FORMAT_BC3_UNORM_BLOCK;
            texFmtToVkFmt[(Uint32)TextureFormat::BC3_sRGB] = VK_FORMAT_BC3_SRGB_BLOCK;

            texFmtToVkFmt[(Uint32)TextureFormat::BC4_UNorm] = VK_FORMAT_BC4_UNORM_BLOCK;
            texFmtToVkFmt[(Uint32)TextureFormat::BC4_SNorm] = VK_FORMAT_BC4_SNORM_BLOCK;

            texFmtToVkFmt[(Uint32)TextureFormat::BC5_UNorm] = VK_FORMAT_BC5_UNORM_BLOCK;
            texFmtToVkFmt[(Uint32)TextureFormat::BC5_SNorm] = VK_FORMAT_BC5_SNORM_BLOCK;

            texFmtToVkFmt[(Uint32)TextureFormat::BC6_UFloat] = VK_FORMAT_BC6H_UFLOAT_BLOCK;
            texFmtToVkFmt[(Uint32)TextureFormat::BC6_SFloat] = VK_FORMAT_BC6H_SFLOAT_BLOCK;

            texFmtToVkFmt[(Uint32)TextureFormat::BC7_UNorm] = VK_FORMAT_BC7_UNORM_BLOCK;
            texFmtToVkFmt[(Uint32)TextureFormat::BC7_sRGB] = VK_FORMAT_BC7_SRGB_BLOCK;

            texFmtToVkFmt[(Uint32)TextureFormat::ETC2_RGB_8_UNorm] = VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
            texFmtToVkFmt[(Uint32)TextureFormat::ETC2_RGB_8_sRGB] = VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK;

            texFmtToVkFmt[(Uint32)TextureFormat::ETC2_RGBA1_8_UNorm] = VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK;
            texFmtToVkFmt[(Uint32)TextureFormat::ETC2_RGBA1_8_sRGB] = VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK;

            texFmtToVkFmt[(Uint32)TextureFormat::ETC2_RGBA_8_UNorm] = VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK;
            texFmtToVkFmt[(Uint32)TextureFormat::ETC2_RGBA_8_sRGB] = VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK;

            texFmtToVkFmt[(Uint32)TextureFormat::EAC_R_11_UNorm] = VK_FORMAT_EAC_R11_UNORM_BLOCK;
            texFmtToVkFmt[(Uint32)TextureFormat::EAC_R_11_SNorm] = VK_FORMAT_EAC_R11_SNORM_BLOCK;

            texFmtToVkFmt[(Uint32)TextureFormat::EAC_RG_11_UNorm] = VK_FORMAT_EAC_R11G11_UNORM_BLOCK;
            texFmtToVkFmt[(Uint32)TextureFormat::EAC_RG_11_SNorm] = VK_FORMAT_EAC_R11G11_SNORM_BLOCK;

            texFmtToVkFmt[(Uint32)TextureFormat::ASTC_4x4_UNorm] = VK_FORMAT_ASTC_4x4_UNORM_BLOCK;
            texFmtToVkFmt[(Uint32)TextureFormat::ASTC_4x4_sRGB] = VK_FORMAT_ASTC_4x4_SRGB_BLOCK;

            texFmtToVkFmt[(Uint32)TextureFormat::ASTC_5x4_UNorm] = VK_FORMAT_ASTC_5x4_UNORM_BLOCK;
            texFmtToVkFmt[(Uint32)TextureFormat::ASTC_5x4_sRGB] = VK_FORMAT_ASTC_5x4_SRGB_BLOCK;

            texFmtToVkFmt[(Uint32)TextureFormat::ASTC_5x5_UNorm] = VK_FORMAT_ASTC_5x5_UNORM_BLOCK;
            texFmtToVkFmt[(Uint32)TextureFormat::ASTC_5x5_sRGB] = VK_FORMAT_ASTC_5x5_SRGB_BLOCK;

            texFmtToVkFmt[(Uint32)TextureFormat::ASTC_6x5_UNorm] = VK_FORMAT_ASTC_6x5_UNORM_BLOCK;
            texFmtToVkFmt[(Uint32)TextureFormat::ASTC_6x5_sRGB] = VK_FORMAT_ASTC_6x5_SRGB_BLOCK;

            texFmtToVkFmt[(Uint32)TextureFormat::ASTC_6x6_UNorm] = VK_FORMAT_ASTC_6x6_UNORM_BLOCK;
            texFmtToVkFmt[(Uint32)TextureFormat::ASTC_6x6_sRGB] = VK_FORMAT_ASTC_6x6_SRGB_BLOCK;

            texFmtToVkFmt[(Uint32)TextureFormat::ASTC_8x5_UNorm] = VK_FORMAT_ASTC_8x5_UNORM_BLOCK;
            texFmtToVkFmt[(Uint32)TextureFormat::ASTC_8x5_sRGB] = VK_FORMAT_ASTC_8x5_SRGB_BLOCK;

            texFmtToVkFmt[(Uint32)TextureFormat::ASTC_8x6_UNorm] = VK_FORMAT_ASTC_8x6_UNORM_BLOCK;
            texFmtToVkFmt[(Uint32)TextureFormat::ASTC_8x6_sRGB] = VK_FORMAT_ASTC_8x6_SRGB_BLOCK;

            texFmtToVkFmt[(Uint32)TextureFormat::ASTC_8x8_UNorm] = VK_FORMAT_ASTC_8x8_UNORM_BLOCK;
            texFmtToVkFmt[(Uint32)TextureFormat::ASTC_8x8_sRGB] = VK_FORMAT_ASTC_8x8_SRGB_BLOCK;

            texFmtToVkFmt[(Uint32)TextureFormat::ASTC_10x5_UNorm] = VK_FORMAT_ASTC_10x5_UNORM_BLOCK;
            texFmtToVkFmt[(Uint32)TextureFormat::ASTC_10x5_sRGB] = VK_FORMAT_ASTC_10x5_SRGB_BLOCK;

            texFmtToVkFmt[(Uint32)TextureFormat::ASTC_10x6_UNorm] = VK_FORMAT_ASTC_10x6_UNORM_BLOCK;
            texFmtToVkFmt[(Uint32)TextureFormat::ASTC_10x6_sRGB] = VK_FORMAT_ASTC_10x6_SRGB_BLOCK;

            texFmtToVkFmt[(Uint32)TextureFormat::ASTC_10x8_UNorm] = VK_FORMAT_ASTC_10x8_UNORM_BLOCK;
            texFmtToVkFmt[(Uint32)TextureFormat::ASTC_10x8_sRGB] = VK_FORMAT_ASTC_10x8_SRGB_BLOCK;

            texFmtToVkFmt[(Uint32)TextureFormat::ASTC_10x10_UNorm] = VK_FORMAT_ASTC_10x10_UNORM_BLOCK;
            texFmtToVkFmt[(Uint32)TextureFormat::ASTC_10x10_sRGB] = VK_FORMAT_ASTC_10x10_SRGB_BLOCK;

            texFmtToVkFmt[(Uint32)TextureFormat::ASTC_12x10_UNorm] = VK_FORMAT_ASTC_12x10_UNORM_BLOCK;
            texFmtToVkFmt[(Uint32)TextureFormat::ASTC_12x10_sRGB] = VK_FORMAT_ASTC_12x10_SRGB_BLOCK;

            texFmtToVkFmt[(Uint32)TextureFormat::ASTC_12x12_UNorm] = VK_FORMAT_ASTC_12x12_UNORM_BLOCK;
            texFmtToVkFmt[(Uint32)TextureFormat::ASTC_12x12_sRGB] = VK_FORMAT_ASTC_12x12_SRGB_BLOCK;
        }
    } // namespace rapi
} // namespace cube
