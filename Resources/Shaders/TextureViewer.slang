import Common;
import StaticSampler;
import Utils;

struct CopyToTextureViewerShaderParameterList
{
    float4 dstSizeAndInvSize;
    Bindless<Texture2D> srcTexture2D;
    Bindless<RWTexture2D> dstTexture;
};

[shader("compute")]
[numthreads(8, 8, 1)]
void CopyToTextureViewer2DCS(
    uint3 threadID : SV_DispatchThreadID,
    ParameterBlock<CopyToTextureViewerShaderParameterList> params
)
{
    const uint2 dstCoord = threadID.xy;
    if (any(dstCoord.xy >= params.dstSizeAndInvSize.xy))
    {
        return;
    }

    const float2 uv = (float2(dstCoord) + 0.5) * params.dstSizeAndInvSize.zw;

    params.dstTexture[dstCoord] = params.srcTexture2D.Sample(GetStaticPointClampSampler(), uv);
}

struct CopyToTextureViewerCubeShaderParameterList
{
    float4 srcSizeAndInvSize;
    Bindless<TextureCube> srcTextureCube;
    Bindless<RWTexture2D> dstTexture;
};

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
    uint3 threadID : SV_DispatchThreadID,
    ParameterBlock<CopyToTextureViewerCubeShaderParameterList> params
)
{
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

struct TextureViewerFetchInfoShaderParameterList
{
    float4 sizeAndInvSize;
    Bindless<Texture2D> canvasTexture;

    int2 positionToRead;
    Bindless<RWBuffer<float>> readbackBuffer;
};

[shader("compute")]
[numthreads(1, 1, 1)]
void TextureViewerFetchInfoCS(
    uint3 threadID : SV_DispatchThreadID,
    ParameterBlock<TextureViewerFetchInfoShaderParameterList> params
)
{
    int2 pos = params.positionToRead;
    if (any(pos < 0) || any(pos >= params.sizeAndInvSize.xy))
    {
        params.readbackBuffer[0] = 0;
        params.readbackBuffer[1] = 0;
        params.readbackBuffer[2] = 0;
        params.readbackBuffer[3] = 0;
        return;
    }

    float4 color = params.canvasTexture.Load(int3(pos, 0));
    params.readbackBuffer[0] = color.x;
    params.readbackBuffer[1] = color.y;
    params.readbackBuffer[2] = color.z;
    params.readbackBuffer[3] = color.w;
}
