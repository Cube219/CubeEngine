#pragma once

#include "Utility/Types.h"
#include "Utility/Flags.h"

namespace cube
{
    namespace rapi
    {
        // Shader
        enum class ShaderType
        {
            Vertex,
            Pixel,
            Geometry,
            Hull,
            Domain,
            Compute
        };

        enum class ShaderLanguage
        {
            HLSL,
            GLSL,
            SPIRV
        };

        // ShaderVariable
        enum class ShaderVariableType
        {
            Constant,
            Storage,
            ConstantTexture,
            StorageTexture,
            Sampler,
            SamplerTexture
        };

        // Pipeline
        enum class PrimitiveTopology
        {
            TriangleList,
            TriangleStrip,
            LineList,
            LineStrip,

            ControlPointPatchList1,
            ControlPointPatchList2,
            ControlPointPatchList3,
            ControlPointPatchList4,
            ControlPointPatchList5,
            ControlPointPatchList6,
            ControlPointPatchList7,
            ControlPointPatchList8,
            ControlPointPatchList9,
            ControlPointPatchList10,
            ControlPointPatchList11,
            ControlPointPatchList12,
            ControlPointPatchList13,
            ControlPointPatchList14,
            ControlPointPatchList15,
            ControlPointPatchList16,
            ControlPointPatchList17,
            ControlPointPatchList18,
            ControlPointPatchList19,
            ControlPointPatchList20,
            ControlPointPatchList21,
            ControlPointPatchList22,
            ControlPointPatchList23,
            ControlPointPatchList24,
            ControlPointPatchList25,
            ControlPointPatchList26,
            ControlPointPatchList27,
            ControlPointPatchList28,
            ControlPointPatchList29,
            ControlPointPatchList30,
            ControlPointPatchList31,
            ControlPointPatchList32,
        };

        // Resource (Buffer, Texture)
        enum class ResourceUsage
        {
            Default,
            Immutable,
            Dynamic,
            Staging
        };

        enum class ResourceMapType
        {
            Read,
            Write,
            ReadWrite
        };

        enum class TextureDimension
        {
            Texture1D,
            Texture1DArray,
            Texture2D,
            Texture2DArray,
            Texture3D,
            TextureCube,
            TextureCubeArray
        };

        enum class TextureViewDimension
        {
            TextureView1D,
            TextureView1DArray,
            TextureView2D,
            TextureView2DArray,
            TextureView3D,
            TextureViewCube,
            TextureViewCubeArray
        };

        enum class TextureViewType
        {
            ShaderResourceView,
            RenderTargetView,
            DepthStencilView,
            UnorderedAccessView
        };

        // TODO: 형식들 정리(안쓰는 것들 제거...)
        enum class TextureFormat : Uint32
        {
            Unknown,

            R_8_Uint,
            R_8_Sint,
            R_8_UNorm,
            R_8_SNorm,
            R_8_sRGB,
            // UScaled, SScaled

            R_16_Float,
            R_16_Uint,
            R_16_Sint,
            R_16_UNorm,
            R_16_SNorm,
            // UScakedm SScaled

            R_32_Float,
            R_32_Uint,
            R_32_Sint,

            R_64_Float,
            R_64_Uint,
            R_64_Sint,

            RG_8_Uint,
            RG_8_Sint,
            RG_8_UNorm,
            RG_8_SNorm,
            RG_8_sRGB,
            // UScaled, SScaled

            RG_16_Float,
            RG_16_Uint,
            RG_16_Sint,
            RG_16_UNorm,
            RG_16_SNorm,
            // UScaled, SScaled

            RG_32_Float,
            RG_32_Uint,
            RG_32_Sint,

            RG_64_Float,
            RG_64_Uint,
            RG_64_Sint,

            RGB_8_Uint,
            RGB_8_Sint,
            RGB_8_UNorm,
            RGB_8_SNorm,
            RGB_8_sRGB,
            // UScaled, SScaled

            RGB_16_Float,
            RGB_16_Uint,
            RGB_16_Sint,
            RGB_16_UNorm,
            RGB_16_SNorm,
            // UScaled, SScaled

            RGB_32_Float,
            RGB_32_Uint,
            RGB_32_Sint,

            RGB_64_Float,
            RGB_64_Uint,
            RGB_64_Sint,

            BGR_8_Uint,
            BGR_8_Sint,
            BGR_8_UNorm,
            BGR_8_SNorm,
            BGR_8_sRGB,
            // UScaled, SScaled

            RGBA_8_Uint,
            RGBA_8_Sint,
            RGBA_8_UNorm,
            RGBA_8_SNorm,
            RGBA_8_sRGB,
            // UScaled, SScaled

            RGBA_16_Float,
            RGBA_16_Uint,
            RGBA_16_Sint,
            RGBA_16_UNorm,
            RGBA_16_SNorm,
            // UScaled, SScaled

            RGBA_32_Float,
            RGBA_32_Uint,
            RGBA_32_Sint,

            RGBA_64_Float,
            RGBA_64_Uint,
            RGBA_64_Sint,

            BGRA_8_Uint,
            BGRA_8_Sint,
            BGRA_8_UNorm,
            BGRA_8_SNorm,
            BGRA_8_sRGB,
            // UScaled, SScaled

            // TODO: Pack32를 붙여하 하나? 무슨 차이?
            // 확인하고 지우거나, 남기거나 하고, VulkanTypeConversion에 내용 추가
            ABGR_8_Uint_Pack32,
            ABGR_8_Sint_Pack32,
            ABGR_8_UNorm_Pack32,
            ABGR_8_SNorm_Pack32,
            ABGR_8_sRGB_Pack32,
            // UScaled, SScaled

            A2RGB_10_Uint_Pack32,
            A2RGB_10_Sint_Pack32,
            A2RGB_10_UNorm_Pack32,
            A2RGB_10_SNorm_Pack32,
            A2RGB_10_sRGB_Pack32,
            // UScaled, SScaled

            A2BGR_10_Uint_Pack32,
            A2BGR_10_Sint_Pack32,
            A2BGR_10_UNorm_Pack32,
            A2BGR_10_SNorm_Pack32,
            A2BGR_10_sRGB_Pack32,
            // UScaled, SScaled

            D16_UNorm,

            D32_Float,

            // Need textureCompressionBC in VkPhysicalDeviceFeatures
            BC1_RGB_UNorm,
            BC1_RGB_sRGB,
            BC1_RGBA_UNorm,
            BC1_RGBA_sRGB,

            BC2_UNorm,
            BC2_sRGB,

            BC3_UNorm,
            BC3_sRGB,

            BC4_UNorm,
            BC4_SNorm,

            BC5_UNorm,
            BC5_SNorm,

            BC6_UFloat,
            BC6_SFloat,

            BC7_UNorm,
            BC7_sRGB,

            // Need textureCompressionETC2 in VkPhysicalDeviceFeatures
            ETC2_RGB_8_UNorm,
            ETC2_RGB_8_sRGB,

            ETC2_RGBA1_8_UNorm,
            ETC2_RGBA1_8_sRGB,

            ETC2_RGBA_8_UNorm,
            ETC2_RGBA_8_sRGB,

            EAC_R_11_UNorm,
            EAC_R_11_SNorm,

            EAC_RG_11_UNorm,
            EAC_RG_11_SNorm,

            // Need textureCompressionASTC_LDR in VkPhysicalDeviceFeatures
            ASTC_4x4_UNorm,
            ASTC_4x4_sRGB,

            ASTC_5x4_UNorm,
            ASTC_5x4_sRGB,

            ASTC_5x5_UNorm,
            ASTC_5x5_sRGB,

            ASTC_6x5_UNorm,
            ASTC_6x5_sRGB,

            ASTC_6x6_UNorm,
            ASTC_6x6_sRGB,

            ASTC_8x5_UNorm,
            ASTC_8x5_sRGB,

            ASTC_8x6_UNorm,
            ASTC_8x6_sRGB,

            ASTC_8x8_UNorm,
            ASTC_8x8_sRGB,

            ASTC_10x5_UNorm,
            ASTC_10x5_sRGB,

            ASTC_10x6_UNorm,
            ASTC_10x6_sRGB,

            ASTC_10x8_UNorm,
            ASTC_10x8_sRGB,

            ASTC_10x10_UNorm,
            ASTC_10x10_sRGB,

            ASTC_12x10_UNorm,
            ASTC_12x10_sRGB,

            ASTC_12x12_UNorm,
            ASTC_12x12_sRGB,

            TextureFormatCount
        };

        enum class TextureBindTypeFlag
        {
            RenderTarget = 1,
            DepthStencil = 2,
            ShaderResource = 4
        };
        using TextureBindTypeFlags = Flags<TextureBindTypeFlag>;
        FLAGS_OPERATOR(TextureBindTypeFlag);

        // RenderPass
        struct ColorClearValue
        {
            float value[4];
        };
        struct DepthStencilValue
        {
            float depth;
            Uint32 stencil;
        };
        struct ClearValue
        {
            bool useColor = true;

            union
            {
                ColorClearValue color;
                DepthStencilValue depthStencil;
            };
        };
    } // namespace rapi
} // namespace cube
