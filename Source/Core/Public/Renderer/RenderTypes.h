#pragma once

#include "CoreHeader.h"

#include "Vector.h"

namespace cube
{
    inline Uint16 FloatToFloat16(float value)
    {
        Uint32 bits;
        memcpy(&bits, &value, sizeof(bits));

        Uint32 sign = (bits >> 31);
        Int32 exponentFP32 = (bits >> 23) & 0xFF;
        Uint32 mantissaFP32 = bits & 0x7FFFFF;

        if (exponentFP32 == 0xFF && mantissaFP32 != 0)
        {
            // NaN
            return static_cast<Uint16>((sign << 15) | 0x7C00 | (mantissaFP32 >> 13));
        }

        Int32 exponentFP16 = exponentFP32 - 127 + 15;
        // Denomalized
        if (exponentFP16 <= 0)
        {
            Uint32 mantissaFP16 = (mantissaFP32 | 0x800000) >> (1 - exponentFP16);
            return static_cast<Uint16>((sign << 15) | (mantissaFP16 >> 13));
        }
        // Overflow to Inf
        else if (exponentFP16 >= 31)
        {
            return static_cast<Uint16>((sign << 15) | 0x7C00);
        }

        return static_cast<Uint16>((sign << 15) | (exponentFP16 << 10) | (mantissaFP32 >> 13));
    }

    struct Vertex
    {
        Vector3 position;
        Vector4 color;
        Vector3 normal;
        Vector4 tangent;
        Vector2 uv;
    };

    struct VertexFP32
    {
        Float3 position; // xyz
        Float4 color;    // rgba
        Float3 normal;   // xyz
        Float4 tangent;  // xyzw
        Float2 uv;       // uv
    };

    struct VertexFP16
    {
        Uint16 position[4]; // xyz + w=1.0
        Uint16 color[4];    // rgba
        Uint16 normal[4];   // xyz + w=0.0
        Uint16 tangent[4];  // xyzw
        Uint16 uv[2];       // uv
    };

    inline VertexFP32 ConvertVertexToFP32(const Vertex& v)
    {
        Float3 pos = v.position.GetFloat3();
        Float4 col = v.color.GetFloat4();
        Float3 nrm = v.normal.GetFloat3();
        Float4 tan = v.tangent.GetFloat4();
        Float2 tex = v.uv.GetFloat2();

        VertexFP32 fp32;
        fp32.position = pos;
        fp32.color = col;
        fp32.normal = nrm;
        fp32.tangent = tan;
        fp32.uv = tex;

        return fp32;
    }

    inline VertexFP16 ConvertVertexToFP16(const Vertex& v)
    {
        Float3 pos = v.position.GetFloat3();
        Float4 col = v.color.GetFloat4();
        Float3 nrm = v.normal.GetFloat3();
        Float4 tan = v.tangent.GetFloat4();
        Float2 tex = v.uv.GetFloat2();

        VertexFP16 fp16;
        fp16.position[0] = FloatToFloat16(pos.x);
        fp16.position[1] = FloatToFloat16(pos.y);
        fp16.position[2] = FloatToFloat16(pos.z);
        fp16.position[3] = FloatToFloat16(1.0f);

        fp16.color[0] = FloatToFloat16(col.x);
        fp16.color[1] = FloatToFloat16(col.y);
        fp16.color[2] = FloatToFloat16(col.z);
        fp16.color[3] = FloatToFloat16(col.w);

        fp16.normal[0] = FloatToFloat16(nrm.x);
        fp16.normal[1] = FloatToFloat16(nrm.y);
        fp16.normal[2] = FloatToFloat16(nrm.z);
        fp16.normal[3] = FloatToFloat16(0.0f);

        fp16.tangent[0] = FloatToFloat16(tan.x);
        fp16.tangent[1] = FloatToFloat16(tan.y);
        fp16.tangent[2] = FloatToFloat16(tan.z);
        fp16.tangent[3] = FloatToFloat16(tan.w);

        fp16.uv[0] = FloatToFloat16(tex.x);
        fp16.uv[1] = FloatToFloat16(tex.y);

        return fp16;
    }

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
