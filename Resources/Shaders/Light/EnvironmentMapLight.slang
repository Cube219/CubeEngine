implementing Light;

import BRDF;
import Common;
import StaticSampler;

public struct EnvironmentMapLightShaderParameterList
{
    public Bindless<TextureCube> diffuseIrradianceMap;
    public Bindless<Texture2D> integratedBRDFLUT;
    public Bindless<TextureCube> prefilterMap;
    public Bindless<SamplerState> prefilterSampler;
    public uint prefilterMapMipLevels;
};

public struct EnvironmentMapLight : ILight
{
    uniform TextureCube diffuseIrradianceMap;
    uniform Texture2D integratedBRDFLUT;
    uniform TextureCube prefilterMap;
    uniform SamplerState prefilterSampler;
    uint prefilterMapMipLevels;

    public __init(
        uniform TextureCube inDiffuseIrradianceMap,
        uniform Texture2D inIntegratedBRDFLUT,
        uniform TextureCube inPrefilterMap,
        uniform SamplerState inPrefilterSampler,
        uint inPrefilterMapMipLevels
    )
    {
        diffuseIrradianceMap = inDiffuseIrradianceMap;
        integratedBRDFLUT = inIntegratedBRDFLUT;
        prefilterMap = inPrefilterMap;
        prefilterSampler = inPrefilterSampler;
        prefilterMapMipLevels = inPrefilterMapMipLevels;
    }

    public float3 CalculateLight(float3 view, float3 normal, float3 worldPosition, MaterialValue materialValue)
    {
        BRDFParameters params = BRDFParameters(normal, view, normal);
#ifdef MATERIAL_PBR
        // Reverse z for right-handed.
        const float3 diffuse = diffuseIrradianceMap.Sample(GetStaticLinearWrapSampler(), float3(normal.xy, -normal.z)).xyz * materialValue.diffuseColor;
        
        const float3 R = -view - 2.0 * dot(normal, -view) * normal;
        const float3 prefilterColor = prefilterMap.SampleLevel(prefilterSampler, float3(R.xy, -R.z), materialValue.roughness * (prefilterMapMipLevels - 1)).xyz;
        const float2 integratedBRDF = integratedBRDFLUT.Sample(GetStaticLinearWrapSampler(), float2(max(params.VoN, 0.0), materialValue.roughness)).xy;
        params.VoH = dot(view, normal);
        
        float3 F0 = materialValue.specularColor;
        const float3 F = BRDF::Fresnel_SchlickRoughness(params.VoH, F0, materialValue.roughness);
        const float3 specular = prefilterColor * (F * integratedBRDF.x + integratedBRDF.y);
        
        const float3 Kd = 1.0f - F;

        return (Kd * diffuse + specular) * materialValue.indirectOcclusion;
#else
        // Reverse z for right-handed.
        return (diffuseIrradianceMap.Sample(GetStaticLinearWrapSampler(), float3(normal.xy, -normal.z)).xyz * materialValue.diffuseColor)
            * materialValue.indirectOcclusion;
#endif
    }
}
