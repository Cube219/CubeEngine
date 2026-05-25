import Common;
import Global;
import "Light/BRDF";
import MonteCarlo;
import StaticSampler;
import Utils;

struct GenerateIrradianceMapShaderParameterList
{
    int numSlices;
    float2 widthAndInvWidth;
    Bindless<TextureCube> srcIBL;
    Bindless<RWTexture2DArray> dstDiffuseIrradianceMap;
};

float3 CalculateDiffuseIrradiance(uniform TextureCube src, float3 normal, uint numSlices)
{
    float3 up = float3(0.0, 1.0, 0.0);
    float3 right = cross(up, normal);
    up = cross(normal, right);

    float3 irradiance = 0.0;
    uint numSamples = 0;
    const float delta = PI * 2.0 / numSlices;
    for (float phi = 0.0; phi < PI * 2.0; phi += delta)
    {
        const float sinP = sin(phi);
        const float cosP = cos(phi);

        for (float theta = 0.0; theta < PI * 0.5; theta += delta)
        {
            const float sinT = sin(theta);
            const float cosT = cos(theta);

            const float3 sampleDir = float3(sinT * cosP, sinT * sinP, cosT);
            const float3 sampleDirWorld = sampleDir.x * right + sampleDir.y * up + sampleDir.z * normal;

            float3 sampledColor = src.Sample(GetStaticLinearWrapSampler(), sampleDirWorld).xyz;
            irradiance += sampledColor * cosT * sinT;
            numSamples++;
        }
    }

    return irradiance * (1.0 / numSamples) * PI;
}

[shader("compute")]
[numthreads(8, 8, 1)]
void GenerateIrradianceMapCS(
    uint3 threadID : SV_DispatchThreadID,
    uint3 groupID : SV_GroupID,
    ParameterBlock<GenerateIrradianceMapShaderParameterList> params
)
{
    const uint3 dstCoord = uint3(threadID.xy, groupID.z);
    if (any(dstCoord.xy >= params.widthAndInvWidth.xx))
    {
        return;
    }

    const float2 uv = (float2(dstCoord.xy) + 0.5) * params.widthAndInvWidth.yy;
    const float3 normal = Cubemap::CubemapUVToVector(uv, dstCoord.z);

    params.dstDiffuseIrradianceMap[dstCoord] = float4(CalculateDiffuseIrradiance(params.srcIBL, normal, params.numSlices), 1);
}

struct GenerateIntegratedBRDFLUTShaderParameterList
{
    uint sampleCount;
    float width;
    Bindless<RWTexture2D> dstIntegratedBRDFLUT;
};

float2 IntegrateBRDF(float NoV, float roughness, uint sampleCount)
{
    const float3 V = float3(sqrt(1.0 - NoV*NoV), 0, NoV);
    const float3 N = float3(0, 0, 1);

    float2 res = 0.0;
    for (uint i = 0; i < sampleCount; ++i)
    {
        const float2 Xi = Hammersley(i, sampleCount);
        const float3 H = ImportanceSampleGGX(Xi, N, roughness);
        const float3 L = normalize(2.0 * dot(V, H) * H - V);

        if (L.z > 0.0)
        {
            BRDFParam param = BRDFParam(L, V, N, 0, 0, roughness, 0, 0);
            const float VoH = max(param.VoH, 0.0);
            const float NoH = max(param.NoH, 0.0);
            const float VoN = max(param.VoN, 0.0);
            float g = Geometry_SmithGGXIBL.Calculate(param);
            float gVis = (g * VoH) / (NoH * VoN);
            float Fc = pow(1.0 - VoH, 5.0);

            res.x += (1.0 - Fc) * gVis;
            res.y += Fc * gVis;
        }
    }
    return res / sampleCount;
}

[shader("compute")]
[numthreads(8, 8, 1)]
void GenerateIntegratedBRDFLUTCS(
    uint3 threadID : SV_DispatchThreadID,
    ParameterBlock<GenerateIntegratedBRDFLUTShaderParameterList> params
)
{
    const uint2 dstCoord = threadID.xy;
    if (any(dstCoord >= params.width.xx))
    {
        return;
    }

    const float2 uv = ((float2)(dstCoord) + 0.5) / params.width.xx;
    params.dstIntegratedBRDFLUT[dstCoord] = float4(IntegrateBRDF(uv.x, uv.y, params.sampleCount), 0);
}

struct GeneratePrefilterMapShaderParameterList
{
    uint numSamples;
    float2 widthAndInvWidth;
    float roughness;
    Bindless<TextureCube> srcIBL;
    Bindless<RWTexture2DArray> dstPrefilterMap;
};

float3 CalculatePrefilter(uniform TextureCube src, float3 normal, float roughness, uint numSamples)
{
    const float3 N = normal;
    const float3 R = N;
    const float3 V = R;

    float totalWeight = 0.0;
    float3 prefilteredColor = 0.0;
    for (uint i = 0; i < numSamples; ++i)
    {
        const float2 Xi = Hammersley(i, numSamples);
        const float3 H = ImportanceSampleGGX(Xi, N, roughness);
        const float3 L = normalize(2.0 * dot(V, H) * H - V);

        const float NoL = max(dot(N, L), 0.0);
        if (NoL > 0.0)
        {
            prefilteredColor += src.Sample(GetStaticLinearWrapSampler(), L).xyz * NoL;
            totalWeight += NoL;
        }
    }

    return prefilteredColor / totalWeight;
}

[shader("compute")]
[numthreads(8, 8, 1)]
void GeneratePrefilterMapCS(
    uint3 threadID : SV_DispatchThreadID,
    uint3 groupID : SV_GroupID,
    ParameterBlock<GeneratePrefilterMapShaderParameterList> params
)
{
    const uint3 dstCoord = uint3(threadID.xy, groupID.z);
    if (any(dstCoord.xy >= params.widthAndInvWidth.xx))
    {
        return;
    }

    const float2 uv = (float2(dstCoord.xy) + 0.5) * params.widthAndInvWidth.yy;
    const float3 normal = Cubemap::CubemapUVToVector(uv, dstCoord.z);

    params.dstPrefilterMap[dstCoord] = float4(CalculatePrefilter(params.srcIBL, normal, params.roughness, params.numSamples), 1);
}
