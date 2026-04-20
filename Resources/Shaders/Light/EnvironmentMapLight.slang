implementing Light;

import BRDF;
import Common;

public struct EnvironmentMapLightShaderParameterList
{
    public Bindless<TextureCube> diffuseIrradianceMap;
    public Bindless<SamplerState> sampler;
};

public struct EnvironmentMapLight : ILight
{
    uniform TextureCube diffuseIrradianceMap;
    uniform SamplerState sampler;

    public __init(uniform TextureCube inDiffuseIrradianceMap, uniform SamplerState inSampler)
    {
        diffuseIrradianceMap = inDiffuseIrradianceMap;
        sampler = inSampler;
    }

    public float3 CalculateLight(float3 view, float3 normal, MaterialValue materialValue)
    {
        // Reverse z for right-handed.
        normal = float3(normal.xy, -normal.z);
#ifdef MATERIAL_PBR
        BRDFParam param = BRDFParam(
            normal, view, normal,
            materialValue.albedo, materialValue.metallic, materialValue.roughness,
            float3(0), 0
        );
        return BRDF.DiffuseWeight(param) * diffuseIrradianceMap.Sample(sampler, normal).xyz * materialValue.albedo;
#else
        BRDFParam param = BRDFParam(
            normal, view, normal,
            materialValue.diffuseColor, 0, 0,
            materialValue.specularColor, materialValue.shininess
        );
        return BlinnPhongBRDF.DiffuseWeight(param) * diffuseIrradianceMap.Sample(sampler, normal).xyz * materialValue.diffuseColor;
#endif
    }
}
