import "../Material";

export public MaterialValue GetMaterialValue(MaterialData_CB materialData, float2 uv)
{
    MaterialValue value;

    if (materialData.useConstantBaseColor != 0)
    {
        value.albedo = materialData.baseColor.rgb;
    }
    else
    {
        // TODO: Check if the texture is valid
        value.albedo = materialData.textureSlot0.Sample(uv).rgb;
    }

    return value;
}
