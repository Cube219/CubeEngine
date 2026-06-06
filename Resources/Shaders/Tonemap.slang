module Tonemap;

import Common;
import GammaCorrection;

// Must match in Tonemap.h
enum TonemapMode
{
    Linear,
    PBRNeutral
};

struct TonemapShaderParameterList
{
    Bindless<Texture2D> srcTexture;
    Bindless<RWTexture2D> dstTexture;

    uint mode;
    bool useGammaCorrect;
};

float3 TonemapPBRNeutral(float3 color)
{
    // https://modelviewer.dev/examples/tone-mapping
    float startCompression = 0.8 - 0.04;
    float desaturation = 0.15;

    float x = min(color.r, min(color.g, color.b));
    float offset = x < 0.08 ? x - 6.25 * x * x : 0.04;
    color -= offset;

    float peak = max(color.r, max(color.g, color.b));
    if (peak < startCompression) return color;

    float d = 1. - startCompression;
    float newPeak = 1. - d * d / (peak + d - startCompression);
    color *= newPeak / peak;

    float g = 1. - 1. / (desaturation * (peak - newPeak) + 1.);
    return lerp(color, newPeak * float3(1, 1, 1), g);
}

[shader("compute")]
[numthreads(8, 8, 1)]
void TonemappingCS(
    uint3 threadId : SV_DispatchThreadID,
    ParameterBlock<TonemapShaderParameterList> params
)
{
    uint2 coord = threadId.xy;
    float4 src = params.srcTexture.Load(int3(coord, 0));

    float3 dst = src.rgb;
    if (params.mode == TonemapMode.Linear)
    {
        dst = src.rgb;
    }
    else if (params.mode == TonemapMode.PBRNeutral)
    {
        dst = TonemapPBRNeutral(src.rgb);
    }

    if (params.useGammaCorrect)
    {
        dst = pow(dst, 1.0 / 2.2);
    }

    params.dstTexture[coord] = float4(dst, src.a);
}

