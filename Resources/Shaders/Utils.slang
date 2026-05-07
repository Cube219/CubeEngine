module Utils;

namespace Cubemap
{
    static float3 baseDirections[6] = {
        float3( 1.0,  0.0,  0.0),
        float3(-1.0,  0.0,  0.0),
        float3( 0.0,  1.0,  0.0),
        float3( 0.0, -1.0,  0.0),
        float3( 0.0,  0.0,  1.0),
        float3( 0.0,  0.0, -1.0)
    };

    static float3 axisOfU[6] = {
        float3( 0.0,  0.0, -1.0),
        float3( 0.0,  0.0,  1.0),
        float3( 1.0,  0.0,  0.0),
        float3( 1.0,  0.0,  0.0),
        float3( 1.0,  0.0,  0.0),
        float3(-1.0,  0.0,  0.0)
    };

    static float3 axisOfV[6] = {
        float3( 0.0, -1.0,  0.0),
        float3( 0.0, -1.0,  0.0),
        float3( 0.0,  0.0,  1.0),
        float3( 0.0,  0.0, -1.0),
        float3( 0.0, -1.0,  0.0),
        float3( 0.0, -1.0,  0.0)
    };

    public float3 CubemapUVToVector(float2 uv, uint faceIndex)
    {
        // [-1, 1]
        const float2 snormUV = uv * 2.0 - 1.0;
        return normalize(
            baseDirections[faceIndex]
            + axisOfU[faceIndex] * snormUV.x
            + axisOfV[faceIndex] * snormUV.y
        );
    }
}
