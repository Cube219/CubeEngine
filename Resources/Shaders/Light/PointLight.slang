implementing Light;

import BRDF;

public struct PointLight : ILight
{
    float3 position;
    float3 intensity;

    public __init(float3 position, float3 intensity)
    {
        this.position = position;
        this.intensity = intensity;
    }

    public float3 CalculateLight(float3 view, float3 normal, float3 worldPosition, MaterialValue materialValue)
    {
        float3 posToLight = position - worldPosition;
        float3 L = normalize(posToLight);

#ifdef MATERIAL_PBR
        BRDFParam param = BRDFParam(
            L, view, normal,
            materialValue.albedo, materialValue.metallic, materialValue.roughness,
            float3(0), 0
        );

        const float3 diffuse = PBRDiffuse.Calculate(param);
        const float3 specular = PBRSpecular.Calculate(param);
        const float3 Kd = PBRSpecular.DiffuseWeight(param);

        float3 v0 = Kd * diffuse + specular;
#else
        BRDFParam param = BRDFParam(
            L, view, normal,
            materialValue.diffuseColor, 0, 0,
            materialValue.specularColor, materialValue.shininess
        );

        const float3 diffuse = LegacyDiffuse.Calculate(param);
        const float3 specular = LegacySpecular.Calculate(param);
        const float3 Kd = LegacySpecular.DiffuseWeight(param);

        float3 v0 = Kd * diffuse + specular;
#endif

        float distanceSq = max(dot(posToLight, posToLight), 0.00001f);
        float attenuation = rcp(distanceSq);

        return v0 * attenuation * intensity;
    }
}
