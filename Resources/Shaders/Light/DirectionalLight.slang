implementing Light;

import BRDF;

public struct DirectionalLight : ILight
{
    float3 direction;

    public __init(float3 dir)
    {
        direction = dir;
    }

    public float3 CalculateLight(float3 view, float3 normal, MaterialValue materialValue)
    {
        BRDFParam param = BRDFParam(
            direction, view, normal,
            materialValue.albedo, materialValue.metallic, materialValue.roughness
        );
        float3 v0 = BRDF.Calculate(param);
        return v0 * dot(normal, direction) * float3(5, 5, 5);
    }
}
