module MainInterface;

public struct GlobalData_CB
{
    public float3 cameraPosition;
    public float4x4 viewProjection;
    public float3 directionalLightDirection;
};

public struct PerObjectData_CB
{
    public float4x4 model;
    public float4x4 modelInverse;
    public float4x4 modelInverseTranspose;
};

public struct VSInput
{
    public float3 position : POSITION;
    public float3 color : COLOR0;
    public float3 normal : NORMAL;
    public float4 tangent : TANGENT;
    public float2 uv : TEXCOORD0;
};

public struct PSInput
{
    public float4 position : SV_POSITION;
    public float3 normal : NORMAL;
    public float4 tangent : TANGENT;
    public float2 uv : TEXCOORD0;
};
