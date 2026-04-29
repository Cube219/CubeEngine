import Common;
import StaticSampler;

struct CopyToTextureViewerShaderParameterList
{
    float4 dstSizeAndInvSize;
    Bindless<Texture2D> srcTexture2D;
    Bindless<RWTexture2D> dstTexture;
};
ParameterBlock<CopyToTextureViewerShaderParameterList> copyToTextureViewerParams;

[shader("compute")]
[numthreads(8, 8, 1)]
void CopyToTextureViewer2DCS(
    uint3 threadID : SV_DispatchThreadID
)
{
    CopyToTextureViewerShaderParameterList params = copyToTextureViewerParams;

    const uint2 dstCoord = threadID.xy;
    if (any(dstCoord.xy >= params.dstSizeAndInvSize.xy))
    {
        return;
    }

    const float2 uv = (float2(dstCoord) + 0.5) * params.dstSizeAndInvSize.zw;

    params.dstTexture[dstCoord] = params.srcTexture2D.Sample(GetStaticPointClampSampler(), uv);
}
