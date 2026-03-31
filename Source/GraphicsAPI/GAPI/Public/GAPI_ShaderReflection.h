#pragma once

#include "GAPIHeader.h"

#include "CubeString.h"

namespace cube
{
    namespace gapi
    {
        struct ShaderParameterReflection
        {
            enum class Type
            {
                Unknown,
                Bool,
                Int,
                Float,
                Float2,
                Float3,
                Float4,
                Matrix,
                Texture,
                Sampler,
                CombinedTextureSampler,
                BindlessHandler
            };

            String name;
            Type type;
            Uint32 offset;
            Uint32 size;
        };
        // TODO: Use formatter style?
        inline const Character* ToString(ShaderParameterReflection::Type type)
        {
            switch (type)
            {
            case ShaderParameterReflection::Type::Unknown:
                return CUBE_T("Unknown");
            case ShaderParameterReflection::Type::Bool:
                return CUBE_T("Bool");
            case ShaderParameterReflection::Type::Int:
                return CUBE_T("Int");
            case ShaderParameterReflection::Type::Float:
                return CUBE_T("Float");
            case ShaderParameterReflection::Type::Float2:
                return CUBE_T("Float2");
            case ShaderParameterReflection::Type::Float3:
                return CUBE_T("Float3");
            case ShaderParameterReflection::Type::Float4:
                return CUBE_T("Float4");
            case ShaderParameterReflection::Type::Matrix:
                return CUBE_T("Matrix");
            case ShaderParameterReflection::Type::Texture:
                return CUBE_T("Texture");
            case ShaderParameterReflection::Type::Sampler:
                return CUBE_T("Sampler");
            case ShaderParameterReflection::Type::CombinedTextureSampler:
                return CUBE_T("CombinedTextureSampler");
            case ShaderParameterReflection::Type::BindlessHandler:
                return CUBE_T("BindlessHandler");
            default:
                break;
            }
            return CUBE_T("Undefined");
        }

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
