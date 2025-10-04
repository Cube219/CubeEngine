import Material;

struct GlobalData_CB
{
    float4x4 viewProjection;
    float3 directionalLightDirection;
};
ConstantBuffer<GlobalData_CB> globalCB;

struct PerObjectData_CB
{
    float4x4 model;
    float4x4 modelInverse;
};
ConstantBuffer<PerObjectData_CB> perObjectCB;

ConstantBuffer<MaterialData_CB> materialCB;

extern struct Material : IMaterial;

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

    Material material;
    MaterialValue materialValue = material.GetMaterialValue(materialCB, input.uv);

    PSOutput output;
    output.color = float4(materialValue.albedo, 1);
    output.color *= intensity;

    return output;
}
