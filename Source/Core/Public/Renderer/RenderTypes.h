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
        enum class Type
        {
            Unknown,
            Texture,
            Sampler,
            CombinedTextureSampler
        };
        Type type = Type::Unknown;
        Uint64 index = -1;
        Uint64 samplerIndex = -1;
    };
    static constexpr BindlessResource InvalidBindlessResource = { BindlessResource::Type::Unknown, static_cast<Uint64>(-1), static_cast<Uint64>(-1) };
} // namespace cube
