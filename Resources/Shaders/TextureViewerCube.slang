import Common;
import StaticSampler;
import Utils;

struct CopyToTextureViewerCubeShaderParameterList
{
    float4 srcSizeAndInvSize;
    Bindless<TextureCube> srcTextureCube;
    Bindless<RWTexture2D> dstTexture;
};
ParameterBlock<CopyToTextureViewerCubeShaderParameterList> copyToTextureViewerCubeParams;

static uint2 cubeCoordMultiplier[6] = {
    uint2(2, 1),
    uint2(0, 1),
    uint2(1, 0),
    uint2(1, 2),
    uint2(1, 1),
    uint2(3, 1),
};

[shader("compute")]
[numthreads(8, 8, 1)]
void CopyToTextureViewerCubeCS(
    uint3 threadID : SV_DispatchThreadID
)
{
    CopyToTextureViewerCubeShaderParameterList params = copyToTextureViewerCubeParams;

    const uint3 srcCoord = threadID.xyz;
    if (any(srcCoord >= float3(params.srcSizeAndInvSize.xy, 6)))
    {
        return;
    }

    const float2 srcUV = (float2(srcCoord.xy) + 0.5) * params.srcSizeAndInvSize.zw;
    const float3 normal = Cubemap::CubemapUVToVector(srcUV, srcCoord.z);
    const uint2 dstCoord = srcCoord.xy + uint2(params.srcSizeAndInvSize.xy) * cubeCoordMultiplier[srcCoord.z];

    params.dstTexture[dstCoord] = params.srcTextureCube.Sample(GetStaticPointClampSampler(), normal);
}
