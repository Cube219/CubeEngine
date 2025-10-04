#pragma once

#include "CoreHeader.h"

#include "Vector.h"

namespace cube
{
    struct Vertex
    {
        Vector3 position;
        Vector4 color;
        Vector3 normal;
        Vector4 tangent;
        Vector2 uv;
    };
    using Index = Uint32;

    struct BindlessResource
    {
        Uint32 index = -1;
        Uint32 samplerIndex = -1;
    };
    static constexpr BindlessResource InvalidBindlessResource = { static_cast<Uint32>(-1), static_cast<Uint32>(-1) };
} // namespace cube
