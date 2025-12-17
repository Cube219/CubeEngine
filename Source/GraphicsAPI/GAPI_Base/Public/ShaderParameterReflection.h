#pragma once

#include "GAPI_BaseHeader.h"

#include "Renderer/ShaderParameter.h"

namespace cube
{
    struct ShaderParameterReflection
    {
        AnsiString name;
        ShaderParameterType type;
        Uint32 offset;
        Uint32 size;
    };

    struct ShaderParameterBlockReflection
    {
        AnsiString typeName;
        Uint32 index;

        Vector<ShaderParameterReflection> params;
    };

    struct ShaderReflection
    {
        Vector<ShaderParameterBlockReflection> blocks;
    };
} // namespace cube
