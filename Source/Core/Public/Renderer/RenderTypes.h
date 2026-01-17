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

    struct BindlessTexture
    {
        Uint64 id = -1;
    };

    struct BindlessSampler
    {
        Uint64 id = -1;
    };

    struct BindlessCombinedTextureSampler
    {
        Uint64 textureId = -1;
        Uint64 samplerId = -1;
    };
} // namespace cube
