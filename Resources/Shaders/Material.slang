module Material;

import Common;
import Global;
import MainInterface;

// Must match in Material.h.
public enum MaterialMode
{
    Opaque,
    Mask
}

public struct MaterialShaderParameterList
{
    public float4 baseColor;
    public float4 diffuseColor;
    public float4 specularColor;
    public float shininess;

    public uint materialMode;
    public float alphaCutoff;

    public Bindless<Texture2D> textureSlot0;
    public Bindless<Texture2D> textureSlot1;
    public Bindless<Texture2D> textureSlot2;
    public Bindless<Texture2D> textureSlot3;
    public Bindless<Texture2D> textureSlot4;

    public bool IsOpaque()
    {
        return materialMode == MaterialMode.Opaque;
    }

    public bool IsMask()
    {
        return materialMode == MaterialMode.Mask;
    }
};

public struct MaterialInput
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
    public float3 emissive = float3(0.0f, 0.0f, 0.0f);
    public float alpha = 1.0f;
    public float indirectOcclusion = 1.0f;
};

public interface IMaterialCode
{
    MaterialInput GetMaterialInput(MaterialShaderParameterList materialParams, PSInput psInput);
}

public struct MaterialValue
{
#ifdef MATERIAL_PBR
    public float3 albedo;
    public float metallic;
    public float roughness;
#else
    public float shininess;
#endif
    public float3 diffuseColor;
    public float3 specularColor;

    public float3 normal;
    public float3 emissive;
    public float alpha;
    public float indirectOcclusion;

    public __init(MaterialInput input)
    {
#ifdef MATERIAL_PBR
        albedo = input.albedo;
        metallic = input.metallic;
        roughness = input.roughness;

        diffuseColor = albedo * (1.0f - metallic);
        specularColor = lerp(0.04f, albedo, metallic);
#else
        diffuseColor = input.diffuseColor;
        specularColor = input.specularColor;
        shininess = input.shininess;
#endif

        normal = input.normal;
        emissive = input.emissive;
        alpha = input.alpha;
        indirectOcclusion = input.indirectOcclusion;
    }
};
