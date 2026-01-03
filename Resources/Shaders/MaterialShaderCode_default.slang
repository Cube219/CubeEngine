import MainInterface;
import Material;

export struct Material : IMaterial
{

MaterialValue GetMaterialValue(MaterialData_CB materialData, PSInput input)
{
    MaterialValue value = {};

    value.albedo = materialData.textureSlot0.Sample(input.uv).rgb;

    return value;
}


}
