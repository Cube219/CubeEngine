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

    public float3 CalculateLight(float3 view, float3 normal, MaterialValue materialValue)
    {
#ifdef MATERIAL_PBR
        BRDFParam param = BRDFParam(
            direction, view, normal,
            materialValue.albedo, materialValue.metallic, materialValue.roughness,
            float3(0), 0
        );

        const float3 diffuse = PBRDiffuse.Calculate(param);
        const float3 specular = PBRSpecular.Calculate(param);
        const float3 Kd = PBRSpecular.DiffuseWeight(param);

        float3 v0 = Kd * diffuse + specular;
#else
        BRDFParam param = BRDFParam(
            direction, view, normal,
            materialValue.diffuseColor, 0, 0,
            materialValue.specularColor, materialValue.shininess
        );
        
        const float3 diffuse = LegacyDiffuse.Calculate(param);
        const float3 specular = LegacySpecular.Calculate(param);
        const float3 Kd = LegacySpecular.DiffuseWeight(param);

        float3 v0 = Kd * diffuse + specular;
#endif
        return v0 * max(dot(normal, direction), 0.0) * intensity;
    }
}
