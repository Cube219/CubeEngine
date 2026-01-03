import MainInterface;
import Material;

export struct Material : IMaterial
{

MaterialValue GetMaterialValue(MaterialData_CB materialData, PSInput input)
{
    MaterialValue value = {};

    value.albedo = materialData.textureSlot0.Sample(input.uv).rgb;
float3 t1 = materialData.textureSlot1.Sample(input.uv).rgb;
value.metallic = t1.g;
value.roughness = t1.b;
float3 t2 = normalize(materialData.textureSlot2.Sample(input.uv).rgb * 2.0f - 1.0f);
value.normal = t2;

    return value;
}


}
