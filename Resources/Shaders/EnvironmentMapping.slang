import Common;
import Global;

struct GenerateIrradianceMapShaderParameterList
{
    int numSlices;
    float2 widthAndInvWidth;
    Bindless<TextureCube> srcIBL;
    Bindless<SamplerState> srcSampler;
    Bindless<RWTexture2DArray> dstDiffuseIrradianceMap;
};

static float3 baseDirections[6] = {
    float3( 1.0,  0.0,  0.0),
    float3(-1.0,  0.0,  0.0),
    float3( 0.0,  1.0,  0.0),
    float3( 0.0, -1.0,  0.0),
    float3( 0.0,  0.0,  1.0),
    float3( 0.0,  0.0, -1.0)
};

static float3 axisOfU[6] = {
    float3( 0.0,  0.0, -1.0),
    float3( 0.0,  0.0,  1.0),
    float3( 1.0,  0.0,  0.0),
    float3( 1.0,  0.0,  0.0),
    float3( 1.0,  0.0,  0.0),
    float3(-1.0,  0.0,  0.0)
};

static float3 axisOfV[6] = {
    float3( 0.0, -1.0,  0.0),
    float3( 0.0, -1.0,  0.0),
    float3( 0.0,  0.0,  1.0),
    float3( 0.0,  0.0, -1.0),
    float3( 0.0, -1.0,  0.0),
    float3( 0.0, -1.0,  0.0)
};

float3 CalculateDiffuseIrradiance(uniform TextureCube src, uniform SamplerState sampler, float3 normal, uint numSlices)
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

            float3 sampledColor = src.Sample(sampler, sampleDirWorld).xyz;
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
    uniform ParameterBlock<GenerateIrradianceMapShaderParameterList> params,
)
{
    const uint3 dstCoord = uint3(threadID.xy, groupID.z);
    if (any(dstCoord.xy >= params.widthAndInvWidth.xx))
    {
        return;
    }

    // [-1, 1]
    const float2 snormUV = (float2(dstCoord.xy) + 0.5) * params.widthAndInvWidth.yy * 2 - 1;
    const float3 normal = normalize(
        baseDirections[dstCoord.z]
        + axisOfU[dstCoord.z] * snormUV.x
        + axisOfV[dstCoord.z] * snormUV.y
    );

    params.dstDiffuseIrradianceMap[dstCoord] = float4(CalculateDiffuseIrradiance(params.srcIBL, params.srcSampler, normal, params.numSlices), 1);
}
