import Common;

struct GenerateMipmapsShaderParameterList
{
    Bindless<Texture2D> srcTexture;
    Bindless<RWTexture2D> dstTexture;
};

[shader("compute")]
[numthreads(8, 8, 1)]
void GenerateMipmapsCS(
    uint3 threadId : SV_DispatchThreadID,
    ParameterBlock<GenerateMipmapsShaderParameterList> params
)
{
    uint2 dstCoord = threadId.xy;
    uint2 srcCoord = dstCoord * 2;

    float4 sum =
        params.srcTexture.Load(int3(srcCoord, 0)) +
        params.srcTexture.Load(int3(srcCoord + uint2(1, 0), 0)) +
        params.srcTexture.Load(int3(srcCoord + uint2(0, 1), 0)) +
        params.srcTexture.Load(int3(srcCoord + uint2(1, 1), 0));

    params.dstTexture[dstCoord] = sum * 0.25;
}
