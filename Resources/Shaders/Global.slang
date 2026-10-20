module Global;

public struct GlobalShaderParameterList
{
    public float3 viewPosition;
    public float4 viewSizeAndInvSize;
    public float4x4 worldToClip;
    public float4x4 clipToWorld;
};
