implementing Light;

public struct DirectionalLight : ILight
{
    float3 direction;

    public __init(float3 dir)
    {
        direction = dir;
    }

    public float3 CalculateLight(float3 view, float3 normal, MaterialValue materialValue)
    {
        return materialValue.albedo * dot(normal, direction);
    }
}
