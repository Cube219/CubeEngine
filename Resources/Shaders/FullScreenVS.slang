module FullScreenVS;

import Common;

public struct FullScreenVSOutput
{
    public float4 position : SV_POSITION;
    public float2 uv : TEXCOORD0;
};

static float4 posAndUV[3] = {
    float4(-1.0, -1.0, 0.0, 1.0),
    float4(3.0, -1.0, 2.0, 1.0),
    float4(-1.0, 3.0, 0.0, -1.0)
};

[shader("vertex")]
FullScreenVSOutput FullScreenVS(uint vertexId : SV_VertexID)
{
    FullScreenVSOutput output;
    output.position = float4(posAndUV[vertexId].xy, 0.0, 1.0);
    output.uv = posAndUV[vertexId].zw;

    return output;
}
