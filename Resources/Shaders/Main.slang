import Light;
import MainInterface;
import Material;

ParameterBlock<GlobalShaderParameters> globalParams;
ParameterBlock<PerObjectShaderParameters> perObjectParams;
ParameterBlock<MaterialShaderParameters> materialParams;

extern struct Material : IMaterial;

struct PSOutput
{
    float4 color;
};

[shader("vertex")]
PSInput VSMain(VSInput input)
{
    PSInput output;

    output.position = mul(mul(float4(input.position, 1), perObjectParams.model), globalParams.viewProjection);
    output.normal = normalize(mul(input.normal, (float3x3)perObjectParams.modelInverseTranspose));
    output.tangent.xyz = normalize(mul(input.tangent.xyz, (float3x3)perObjectParams.model));
    output.tangent.w = input.tangent.w;
    output.uv = input.uv;

    return output;
}

[shader("pixel")]
PSOutput PSMain(PSInput input) : SV_TARGET
{
    Material material;
    MaterialValue materialValue = material.GetMaterialValue(materialParams, input);

    float3 normal = input.normal;
    float3 tangent = input.tangent.xyz;
    float3 bitangent = cross(normal, tangent) * input.tangent.w;

    float3x3 TBN = float3x3(tangent, bitangent, normal);
    normal = mul(materialValue.normal, TBN);

    float3 view = normalize(globalParams.cameraPosition - input.position.xyz);

    ILight directionalLight = DirectionalLight(normalize(globalParams.directionalLightDirection));
    float3 color = directionalLight.CalculateLight(view, normal, materialValue);

    PSOutput output;
    output.color = float4(color, 1);

    return output;
}
