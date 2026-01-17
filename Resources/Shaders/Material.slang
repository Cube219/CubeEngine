module Material;

import Common;
import MainInterface;

public struct MaterialData_CB
{
    public float4 baseColor;

    public Bindless<Sampler2D> textureSlot0;
    public Bindless<Sampler2D> textureSlot1;
    public Bindless<Sampler2D> textureSlot2;
    public Bindless<Sampler2D> textureSlot3;
    public Bindless<Sampler2D> textureSlot4;
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
    MaterialValue GetMaterialValue(MaterialData_CB materialData, PSInput input);
}
