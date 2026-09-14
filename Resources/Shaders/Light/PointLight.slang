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

        BRDFParameters params = BRDFParameters(L, view, normal);
#ifdef MATERIAL_PBR
        const float3 diffuse = BRDF::Diffuse_Lambert(materialValue.diffuseColor);
        const float3 specular = BRDF::Specular_CookTorrance(params, materialValue.specularColor, materialValue.roughness);
        float3 Kd = 1.0f - BRDF::Fresnel_Schlick(params.VoH, materialValue.specularColor);

        float3 v0 = Kd * diffuse + specular;
#else
        const float3 diffuse = BRDF::Diffuse_Lambert(materialValue.diffuseColor);
        const float3 specular = BRDF::Specular_BlinnPhong(params, materialValue.specularColor, materialValue.shininess);

        float3 v0 = diffuse + specular;
#endif

        float distanceSq = max(dot(posToLight, posToLight), 0.00001f);
        float attenuation = rcp(distanceSq);

        return v0 * max(dot(normal, L), 0.0) * attenuation * intensity;
    }
}
