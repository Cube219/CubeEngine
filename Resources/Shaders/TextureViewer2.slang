import Common;

struct TextureViewerFetchInfoShaderParameterList
{
    float4 sizeAndInvSize;
    Bindless<Texture2D> copiedTexture;

    int positionToReadX;
    int positionToReadY;
    Bindless<RWBuffer<float>> readbackBuffer;
};
ParameterBlock<TextureViewerFetchInfoShaderParameterList> fetchInfoParams;

[shader("compute")]
[numthreads(1, 1, 1)]
void TextureViewerFetchInfoCS(
    uint3 threadID : SV_DispatchThreadID
)
{
    TextureViewerFetchInfoShaderParameterList params = fetchInfoParams;

    int2 pos = int2(params.positionToReadX, params.positionToReadY);
    if (any(pos < 0) || any(pos >= params.sizeAndInvSize.xy))
    {
        params.readbackBuffer[0] = 0;
        params.readbackBuffer[1] = 0;
        params.readbackBuffer[2] = 0;
        params.readbackBuffer[3] = 0;
        return;
    }

    float4 color = params.copiedTexture.Load(int3(pos, 0));
    params.readbackBuffer[0] = color.x;
    params.readbackBuffer[1] = color.y;
    params.readbackBuffer[2] = color.z;
    params.readbackBuffer[3] = color.w;
}
