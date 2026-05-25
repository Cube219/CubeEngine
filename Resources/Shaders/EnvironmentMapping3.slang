import "Light/BRDF";
import Common;
import Global;
import MonteCarlo;
import StaticSampler;
import Utils;

struct GeneratePrefilterMapShaderParameterList
{
    uint numSamples;
    float2 widthAndInvWidth;
    float roughness;
    Bindless<TextureCube> srcIBL;
    Bindless<RWTexture2DArray> dstPrefilterMap;
};
ParameterBlock<GeneratePrefilterMapShaderParameterList> generatePrefilterMapShaderParams;

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

// TODO: Use entrypoint parameter and merge EnvironmentMapping files.
[shader("compute")]
[numthreads(8, 8, 1)]
void GeneratePrefilterMapCS(
    uint3 threadID : SV_DispatchThreadID,
    uint3 groupID : SV_GroupID
)
{
    GeneratePrefilterMapShaderParameterList params = generatePrefilterMapShaderParams;

    const uint3 dstCoord = uint3(threadID.xy, groupID.z);
    if (any(dstCoord.xy >= params.widthAndInvWidth.xx))
    {
        return;
    }

    const float2 uv = (float2(dstCoord.xy) + 0.5) * params.widthAndInvWidth.yy;
    const float3 normal = Cubemap::CubemapUVToVector(uv, dstCoord.z);

    params.dstPrefilterMap[dstCoord] = float4(CalculatePrefilter(params.srcIBL, normal, params.roughness, params.numSamples), 1);
}
