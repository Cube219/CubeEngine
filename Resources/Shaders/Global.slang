module Global;

public struct GlobalShaderParameterList
{
    public float3 viewPosition;
    public float4x4 viewProjection;
    public bool isDirectionalLightEnabled;
    public float3 directionalLightDirection;
    public float3 directionalLightIntensity;
};
