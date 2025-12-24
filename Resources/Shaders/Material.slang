module Material;

import MainInterface;

public struct MaterialShaderParameters
{
    public float4 baseColor;

    public Sampler2D.Handle textureSlot0;
    public Sampler2D.Handle textureSlot1;
    public Sampler2D.Handle textureSlot2;
    public Sampler2D.Handle textureSlot3;
    public Sampler2D.Handle textureSlot4;
};

public struct MaterialValue
{
    public float3 albedo = float3(1.0f, 1.0f, 1.0f);
    public float metallic = 0.0f;
    public float roughness = 1.0f;
    public float3 normal = float3(0.0f, 0.0f, 1.0f);
};

public interface IMaterial
{
    MaterialValue GetMaterialValue(MaterialShaderParameters materialParams, PSInput input);
}
