module Material;

// Shader variables
public struct MaterialData_CB
{
    public int useConstantBaseColor;
    public float4 baseColor;

    public Sampler2D.Handle textureSlot0;
    public Sampler2D.Handle textureSlot1;
    public Sampler2D.Handle textureSlot2;
    public Sampler2D.Handle textureSlot3;
    public Sampler2D.Handle textureSlot4;
};

public struct MaterialValue
{
    public float3 albedo;
    public float metallic;
    public float roughness;
    public float3 normal;
};

extern public MaterialValue GetMaterialValue(MaterialData_CB materialData, float2 uv);
