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
        BRDFParam param = BRDFParam(
            direction, view, normal,
            materialValue.albedo, materialValue.metallic, materialValue.roughness
        );
        float3 v0 = BRDF.Calculate(param);
        return v0 * dot(normal, direction) * intensity;
    }
}
