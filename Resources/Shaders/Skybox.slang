import Common;
import Global;
import MainInterface;
import StaticSampler;

struct SkyboxShaderParameterList
{
    Bindless<TextureCube> skyboxTexture;
    Bindless<ByteAddressBuffer> vertexBuffer;
    bool useFP16;
};
ParameterBlock<GlobalShaderParameterList> globalParams;
ParameterBlock<SkyboxShaderParameterList> skyboxParams;

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
    Vertex v = Vertex(skyboxParams.vertexBuffer, vertexId, skyboxParams.useFP16);

    SkyboxPSInput output;
    output.position = mul(float4(v.position + globalParams.viewPosition, 1), globalParams.viewProjection);
    // Set z as 0 for early-z optimization.
    output.position.z = 0;
    // Reverse z to convert right-handed.
    output.uvw = float3(v.position.x, v.position.y, -v.position.z);

    return output;
}

[shader("pixel")]
SkyboxPSOutput PSMain(SkyboxPSInput input) : SV_TARGET
{
    SkyboxPSOutput output;
    output.color = skyboxParams.skyboxTexture.Sample(GetStaticLinearWrapSampler(), input.uvw);

    return output;
}
