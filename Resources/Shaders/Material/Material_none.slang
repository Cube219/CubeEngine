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
        value.albedo = float3(1, 1, 1);
    }

    return value;
}
