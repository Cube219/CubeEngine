module Material;

// Shader variables
public struct MaterialData_CB
{
    public float4 baseColor;

    public Sampler2D.Handle textureSlot0;
    public Sampler2D.Handle textureSlot1;
    public Sampler2D.Handle textureSlot2;
    public Sampler2D.Handle textureSlot3;
    public Sampler2D.Handle textureSlot4;
};

export public struct MaterialValue
{
    public float3 albedo = float3(1.0f, 1.0f, 1.0f);
    public float metallic = 0.0f;
    public float roughness = 1.0f;
    public float3 normal = float3(0.0f, 0.0f, 1.0f);
};

extern public MaterialValue GetMaterialValue(MaterialData_CB materialData, float2 uv);
