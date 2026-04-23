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
SkyboxPSInput VSMain(VSInput input)
{
    SkyboxPSInput output;
    output.position = mul(float4(input.position + globalParams.viewPosition, 1), globalParams.viewProjection);
    // Set z as 0 for early-z optimization.
    output.position.z = 0;
    // Reverse z to convert right-handed.
    output.uvw = float3(input.position.x, input.position.y, -input.position.z);

    return output;
}

[shader("pixel")]
SkyboxPSOutput PSMain(SkyboxPSInput input) : SV_TARGET
{
    SkyboxPSOutput output;
    output.color = skyboxParams.skyboxTexture.Sample(GetStaticLinearWrapSampler(), input.uvw);

    float gamma = 2.2;
    output.color = float4(pow(output.color.xyz, float3(1.0/gamma)), 1);

    return output;
}
