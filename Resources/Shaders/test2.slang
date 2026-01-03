
struct MaterialData_CB
{
    float4 baseColor;

    DescriptorHandle<Sampler2D> textureSlot0;
    DescriptorHandle<Texture2D> textureSlot1;
};
ParameterBlock<MaterialData_CB> materialCB;

struct PSInput
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float4 tangent : TANGENT;
    float2 uv : TEXCOORD0;
};

float3 GetAlbedo(MaterialData_CB materialData, PSInput input)
{
    return materialData.textureSlot0.Sample(input.uv).rgb;
}

[shader("pixel")]
float4 PSMain(PSInput input) : SV_TARGET
{
    float3 albedo = GetAlbedo(materialCB, input);
    //float3 albedo = materialCB.textureSlot0.Sample(input.uv).rgb;
    //int2 uvInt = int2(input.uv);
    //float3 albedo2 = materialCB.textureSlot1.Load(int3(uvInt, 1)).rgb;

    return float4(albedo, 1.0f);
}

