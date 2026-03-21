module Material;

import Common;
import MainInterface;

public struct MaterialData_CB
{
    public float4 baseColor;
    public float4 diffuseColor;
    public float4 specularColor;
    public float shininess;

    public Bindless<Sampler2D> textureSlot0;
    public Bindless<Sampler2D> textureSlot1;
    public Bindless<Sampler2D> textureSlot2;
    public Bindless<Sampler2D> textureSlot3;
    public Bindless<Sampler2D> textureSlot4;
};

public struct MaterialValue
{
#ifdef MATERIAL_PBR
    public float3 albedo = float3(1.0f, 1.0f, 1.0f);
    public float metallic = 0.0f;
    public float roughness = 1.0f;
#else
    public float3 diffuseColor = float3(1.0f, 1.0f, 1.0f);
    public float3 specularColor = float3(0.0f, 0.0f, 0.0f);
    public float shininess = 0.0f;
#endif
    public float3 normal = float3(0.0f, 0.0f, 1.0f);
};

public interface IMaterial
{
    MaterialValue GetMaterialValue(MaterialData_CB materialData, PSInput input);
}
