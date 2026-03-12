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
        float3 v0 = BRDF.Calculate(param);
#else
        BRDFParam param = BRDFParam(
            direction, view, normal,
            materialValue.diffuseColor, 0, 0,
            materialValue.specularColor, materialValue.shininess
        );
        float3 v0 = BlinnPhongBRDF.Calculate(param);
#endif
        return v0 * dot(normal, direction) * intensity;
    }
}
