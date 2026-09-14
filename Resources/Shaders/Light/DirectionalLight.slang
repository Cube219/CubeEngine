implementing Light;

import BRDF;

public struct DirectionalLight : ILight
{
    float3 direction;
    float3 intensity;

    public __init(float3 inDirection, float3 inIntensity)
    {
        direction = inDirection;
        intensity = inIntensity;
    }

    public float3 CalculateLight(float3 view, float3 normal, float3 worldPosition, MaterialValue materialValue)
    {
        BRDFParameters params = BRDFParameters(direction, view, normal);
#ifdef MATERIAL_PBR
        const float3 diffuse = BRDF::Diffuse_Lambert(materialValue.diffuseColor);
        const float3 specular = BRDF::Specular_CookTorrance(params, materialValue.specularColor, materialValue.roughness);
        const float3 Kd = 1.0f - BRDF::Fresnel_Schlick(params.VoH, materialValue.specularColor);

        float3 v0 = Kd * diffuse + specular;
#else
        const float3 diffuse = BRDF::Diffuse_Lambert(materialValue.diffuseColor);
        const float3 specular = BRDF::Specular_BlinnPhong(params, materialValue.specularColor, materialValue.shininess);

        float3 v0 = diffuse + specular;
#endif
        return v0 * max(dot(normal, direction), 0.0) * intensity;
    }
}
