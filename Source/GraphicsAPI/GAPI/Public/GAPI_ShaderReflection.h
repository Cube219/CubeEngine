#pragma once

#include "GAPIHeader.h"

#include "CubeString.h"

namespace cube
{
    namespace gapi
    {
        // Note: Also add in SlangHelperPrivate::GetReflection.
        enum class ShaderParameterType
        {
            Unknown,
            Bool,
            Int,
            Float,
            Float2,
            Float3,
            Float4,
            Matrix,
            BindlessTexture,
            BindlessSampler,
            BindlessCombinedTextureSampler,
            RGTextureSRV,
            RGTextureUAV,
        };

        struct ShaderParameterReflection
        {
            String name;
            ShaderParameterType type;
            Uint32 offset;
            Uint32 size;
        };

        struct ShaderParameterBlockReflection
        {
            String typeName;
            Uint32 index;

            Vector<ShaderParameterReflection> params;
        };

        struct ShaderReflection
        {
            String name;

            Vector<ShaderParameterBlockReflection> blocks;

            Uint32 threadGroupSizeX = 0;
            Uint32 threadGroupSizeY = 0;
            Uint32 threadGroupSizeZ = 0;
        };
    } // namespace gapi
} // namespace cube
