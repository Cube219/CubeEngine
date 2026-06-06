import Common;
import Global;
import MainInterface;
import StaticSampler;

struct SkyboxShaderParameterList
{
    Bindless<TextureCube> skyboxTexture;
};
ParameterBlock<GlobalShaderParameterList> globalParams;
ParameterBlock<SkyboxShaderParameterList> skyboxParams;

static float3 boxVertexPosition[] = {
    float3(-0.5, -0.5, -0.5),
    float3(-0.5, 0.5, -0.5),
    float3(0.5, -0.5, -0.5),
    float3(0.5, 0.5, -0.5),
    float3(0.5, -0.5, 0.5),
    float3(0.5, 0.5, 0.5),
    float3(-0.5, -0.5, 0.5),
    float3(-0.5, 0.5, 0.5),
};

static uint boxIndices[] = {
    0,1,2,  2,1,3,  2,3,4,  4,3,5,  4,5,6,  6,5,7,  6,7,0,  0,7,1,  6,0,2,  2,4,6,  3,1,5,  5,1,7
};

struct SkyboxPSInput
{
    float4 position : SV_POSITION;
    float3 uvw : TEXCOORD0;
};

struct SkyboxPSOutput
{
    float4 color;
};

[shader("vertex")]
SkyboxPSInput VSMain(uint vertexId : SV_VertexID)
{
    uint vertexIndex = boxIndices[vertexId];
    float3 position = boxVertexPosition[vertexIndex];

    SkyboxPSInput output;
    output.position = mul(float4(position + globalParams.viewPosition, 1), globalParams.viewProjection);
    // Set z to 0 for early-z optimization.
    output.position.z = 0;
    // Reverse z to convert to right-handed coordinate.
    output.uvw = float3(position.x, position.y, -position.z);

    return output;
}

[shader("pixel")]
SkyboxPSOutput PSMain(SkyboxPSInput input) : SV_TARGET
{
    SkyboxPSOutput output;
    output.color = skyboxParams.skyboxTexture.Sample(GetStaticLinearWrapSampler(), input.uvw);

    return output;
}
