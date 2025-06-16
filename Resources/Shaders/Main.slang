struct GlobalData
{
    float4x4 viewProjection;
    float3 directionalLightDirection;
};
ConstantBuffer<GlobalData> globalCB;

struct PerObjectData
{
    float4x4 model;
    float4x4 modelInverse;
};
ConstantBuffer<PerObjectData> perObjectCB;

struct MaterialData
{
    float4 baseColor;
    int useBaseColorTexture;
    DescriptorHandle<Texture2D> baseColorTexture;
    DescriptorHandle<SamplerState> sampler;
};
ConstantBuffer<MaterialData> materialCB;

struct VSInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
};

struct PSOutput
{
    float4 color;
};

[shader("vertex")]
PSInput VSMain(VSInput input)
{
    PSInput output;

    output.position = mul(mul(float4(input.position, 1), perObjectCB.model), globalCB.viewProjection);
    output.normal = input.normal;
    output.uv = input.uv;

    return output;
}

[shader("pixel")]
PSOutput PSMain(PSInput input) : SV_TARGET
{
    float3 ligthDir = normalize(mul(globalCB.directionalLightDirection, (float3x3)perObjectCB.modelInverse));
    float intensity = dot(input.normal, ligthDir);

    PSOutput output;
    if (materialCB.useBaseColorTexture != 0)
    {
        output.color = materialCB.baseColorTexture.Sample(
            materialCB.sampler,
            input.uv
        );
    }
    else
    {
        output.color = materialCB.baseColor;
    }
    output.color *= intensity;

    return output;
}
