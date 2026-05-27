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

    public float3 CalculateLight(float3 view, float3 normal, MaterialValue materialValue)
    {
#ifdef MATERIAL_PBR
        BRDFParam param = BRDFParam(
            normal, view, normal,
            materialValue.albedo, materialValue.metallic, materialValue.roughness,
            float3(0), 0
        );

        // Reverse z for right-handed.
        const float3 diffuse = diffuseIrradianceMap.Sample(GetStaticLinearWrapSampler(), float3(normal.xy, -normal.z)).xyz * materialValue.albedo;
        
        const float3 R = -view - 2.0 * dot(normal, -view) * normal;
        const float3 prefilterColor = prefilterMap.SampleLevel(prefilterSampler, float3(R.xy, -R.z), param.roughness * prefilterMapMipLevels).xyz;
        const float2 integratedBRDF = integratedBRDFLUT.Sample(GetStaticLinearWrapSampler(), float2(max(param.VoN, 0.0), param.roughness)).xy;
        param.VoH = dot(view, normal);
        const float3 F = Fresnel_SchlickRoughness.Calculate(param);
        const float3 specular = prefilterColor * (F * integratedBRDF.x + integratedBRDF.y);
        
        const float3 Kd = PBRSpecular.DiffuseWeight(param);

        return (Kd * diffuse + specular) * materialValue.indirectOcclusion;
#else
        BRDFParam param = BRDFParam(
            normal, view, normal,
            materialValue.diffuseColor, 0, 0,
            materialValue.specularColor, materialValue.shininess
        );

        const float3 Kd = 1.0;
        // Reverse z for right-handed.
        return (Kd * diffuseIrradianceMap.Sample(GetStaticLinearWrapSampler(), float3(normal.xy, -normal.z)).xyz * materialValue.diffuseColor)
            * materialValue.indirectOcclusion;
#endif
    }
}
