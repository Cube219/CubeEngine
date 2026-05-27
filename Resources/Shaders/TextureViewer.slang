import Common;
import StaticSampler;
import Utils;

struct CopyToTextureViewerShaderParameterList
{
    float4 dstSizeAndInvSize;
    Bindless<Texture2D> srcTexture2D;
    Bindless<RWTexture2D> dstTexture;
    uint4 rgbaMask;
    float2 colorRange;
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

    float4 color = params.srcTexture2D.Sample(GetStaticPointClampSampler(), uv);
    uint4 mask = params.rgbaMask;
    if (mask.x == 0 && mask.y == 0 && mask.z == 0 && mask.w != 0)
    {
        color = float4(color.a, color.a, color.a, 1.0);
    }
    else
    {
        color.rgb *= float3(mask.xyz);
        color.a = mask.w > 0 ? color.a : 1.0;
    }
    {
        float rangeDenom = params.colorRange.y - params.colorRange.x;
        if (rangeDenom > 0.0)
        {
            color.rgb = saturate((color.rgb - params.colorRange.x) / rangeDenom);
        }
    }
    params.dstTexture[dstCoord] = color;
}

struct CopyToTextureViewerCubeShaderParameterList
{
    float4 srcSizeAndInvSize;
    Bindless<TextureCube> srcTextureCube;
    Bindless<RWTexture2D> dstTexture;
    uint4 rgbaMask;
    float2 colorRange;
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

    float4 color = params.srcTextureCube.Sample(GetStaticPointClampSampler(), normal);
    uint4 mask = params.rgbaMask;
    if (mask.x == 0 && mask.y == 0 && mask.z == 0 && mask.w != 0)
    {
        color = float4(color.a, color.a, color.a, 1.0);
    }
    else
    {
        color.rgb *= float3(mask.xyz);
        color.a = mask.w > 0 ? color.a : 1.0;
    }
    {
        float rangeDenom = params.colorRange.y - params.colorRange.x;
        if (rangeDenom > 0.0)
        {
            color.rgb = saturate((color.rgb - params.colorRange.x) / rangeDenom);
        }
    }
    params.dstTexture[dstCoord] = color;
}

struct TextureViewerFetchInfo2DShaderParameterList
{
    float4 sizeAndInvSize;
    Bindless<Texture2D> srcTexture2D;
    int2 positionToRead;
    Bindless<RWBuffer<float>> readbackBuffer;
};

[shader("compute")]
[numthreads(1, 1, 1)]
void TextureViewerFetchInfo2DCS(
    uint3 threadID : SV_DispatchThreadID,
    ParameterBlock<TextureViewerFetchInfo2DShaderParameterList> params
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

    float2 uv = (float2(pos) + 0.5) * params.sizeAndInvSize.zw;
    float4 color = params.srcTexture2D.Sample(GetStaticPointClampSampler(), uv);
    params.readbackBuffer[0] = color.x;
    params.readbackBuffer[1] = color.y;
    params.readbackBuffer[2] = color.z;
    params.readbackBuffer[3] = color.w;
}

struct TextureViewerFetchInfoCubeShaderParameterList
{
    float4 faceSizeAndInvSize;
    Bindless<TextureCube> srcTextureCube;
    int2 positionToRead;
    Bindless<RWBuffer<float>> readbackBuffer;
};

[shader("compute")]
[numthreads(1, 1, 1)]
void TextureViewerFetchInfoCubeCS(
    uint3 threadID : SV_DispatchThreadID,
    ParameterBlock<TextureViewerFetchInfoCubeShaderParameterList> params
)
{
    int2 pos = params.positionToRead;
    uint2 faceSize = uint2(params.faceSizeAndInvSize.xy);

    int faceIndex = -1;
    uint2 facePixel = uint2(0, 0);
    for (int i = 0; i < 6; i++)
    {
        uint2 faceMin = faceSize * cubeCoordMultiplier[i];
        if (pos.x >= int(faceMin.x) && pos.x < int(faceMin.x + faceSize.x) &&
            pos.y >= int(faceMin.y) && pos.y < int(faceMin.y + faceSize.y))
        {
            faceIndex = i;
            facePixel = uint2(pos) - faceMin;
            break;
        }
    }

    if (faceIndex < 0)
    {
        params.readbackBuffer[0] = 0;
        params.readbackBuffer[1] = 0;
        params.readbackBuffer[2] = 0;
        params.readbackBuffer[3] = 0;
        return;
    }

    float2 faceUV = (float2(facePixel) + 0.5) * params.faceSizeAndInvSize.zw;
    float3 normal = Cubemap::CubemapUVToVector(faceUV, faceIndex);
    float4 color = params.srcTextureCube.Sample(GetStaticPointClampSampler(), normal);
    params.readbackBuffer[0] = color.x;
    params.readbackBuffer[1] = color.y;
    params.readbackBuffer[2] = color.z;
    params.readbackBuffer[3] = color.w;
}
