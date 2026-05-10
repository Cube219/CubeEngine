module MainInterface;

import Common;

public struct Vertex
{
    public float3 position;
    public float4 color;
    public float3 normal;
    public float4 tangent;
    public float2 uv;

    public __init(ByteAddressBuffer buffer, uint index, bool useFP16)
    {
        if (useFP16)
        {
            uint offset = (sizeof(half4) * 4 + sizeof(half2)) * index;
            position = buffer.Load<half3>(offset);
            color = buffer.Load<half4>(offset + 8);
            normal = buffer.Load<half3>(offset + 16);
            tangent = buffer.Load<half4>(offset + 24);
            uv = buffer.Load<half2>(offset + 32);
        }
        else
        {
            uint offset = (sizeof(float4) * 4 + sizeof(float2)) * index;
            position = buffer.Load<float3>(offset);
            color = buffer.Load<float4>(offset + 16);
            normal = buffer.Load<float3>(offset + 32);
            tangent = buffer.Load<float4>(offset + 48);
            uv = buffer.Load<float2>(offset + 64);
        }
    }
}

public struct ObjectShaderParameterList
{
    public float4x4 model;
    public float4x4 modelInverse;
    public float4x4 modelInverseTranspose;
    public Bindless<ByteAddressBuffer> vertexBuffer;
    public bool useFP16;
};

public struct SubMeshShaderParameterList
{
    public int vertexBufferOffset;
};

public struct PSInput
{
    public float4 position : SV_POSITION;
    public float4 worldPosition : POSITION;
    public float3 normal : NORMAL;
    public float4 tangent : TANGENT;
    public float2 uv : TEXCOORD0;
};
