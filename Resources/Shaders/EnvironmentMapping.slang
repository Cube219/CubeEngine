import Common;
import Global;
import StaticSampler;
import Utils;

struct GenerateIrradianceMapShaderParameterList
{
    int numSlices;
    float2 widthAndInvWidth;
    Bindless<TextureCube> srcIBL;
    Bindless<RWTexture2DArray> dstDiffuseIrradianceMap;
};
ParameterBlock<GenerateIrradianceMapShaderParameterList> generateIrradianceMapParams;

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
)
{
    GenerateIrradianceMapShaderParameterList params = generateIrradianceMapParams;

    const uint3 dstCoord = uint3(threadID.xy, groupID.z);
    if (any(dstCoord.xy >= params.widthAndInvWidth.xx))
    {
        return;
    }

    const float2 uv = (float2(dstCoord.xy) + 0.5) * generateIrradianceMapParams.widthAndInvWidth.yy;
    const float3 normal = Cubemap::CubemapUVToVector(uv, dstCoord.z);

    params.dstDiffuseIrradianceMap[dstCoord] = float4(CalculateDiffuseIrradiance(params.srcIBL, normal, params.numSlices), 1);
}
