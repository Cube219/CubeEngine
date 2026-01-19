import Light;
import MainInterface;
import Material;

ParameterBlock<GlobalData_CB> globalCB;
ParameterBlock<PerObjectData_CB> perObjectCB;
ParameterBlock<MaterialData_CB> materialCB;

extern struct Material : IMaterial;

struct PSOutput
{
    float4 color;
};

[shader("vertex")]
PSInput VSMain(VSInput input)
{
    PSInput output;

    output.position = mul(mul(float4(input.position, 1), perObjectCB.model), globalCB.viewProjection);
    output.worldPosition = mul(float4(input.position, 1), perObjectCB.model);
    output.normal = normalize(mul(input.normal, (float3x3)perObjectCB.modelInverseTranspose));
    output.tangent.xyz = normalize(mul(input.tangent.xyz, (float3x3)perObjectCB.model));
    output.tangent.w = input.tangent.w;
    output.uv = input.uv;

    return output;
}

[shader("pixel")]
PSOutput PSMain(PSInput input) : SV_TARGET
{
    Material material;
    MaterialValue materialValue = material.GetMaterialValue(materialCB, input);

    float3 normal = input.normal;
    float3 tangent = input.tangent.xyz;
    float3 bitangent = normalize(cross(normal, tangent) * input.tangent.w);

    float3x3 TBN = float3x3(tangent, bitangent, normal);
    normal = normalize(mul(materialValue.normal, TBN));

    float3 view = normalize(globalCB.cameraPosition - input.worldPosition.xyz);

    ILight directionalLight = DirectionalLight(normalize(globalCB.directionalLightDirection), globalCB.directionalLightIntensity);
    float3 color = directionalLight.CalculateLight(view, normal, materialValue);

    PSOutput output;
    output.color = float4(color, 1);

    return output;
}
