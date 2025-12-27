struct GenerateMipmapsParameters
{
    DescriptorHandle<Texture2D> srcTexture;
    DescriptorHandle<RWTexture2D> dstTexture;
};
ParameterBlock<GenerateMipmapsParameters> generateMipmapsParams;

[shader("compute")]
[numthreads(8, 8, 1)]
void CSMain(uint3 threadId : SV_DispatchThreadID)
{
    Texture2D srcTexture = generateMipmapsParams.srcTexture;
    RWTexture2D dstTexture = generateMipmapsParams.dstTexture;

    uint2 dstCoord = threadId.xy;
    uint2 srcCoord = dstCoord * 2;


    float4 sum =
        srcTexture.Load(int3(srcCoord, 0)) +
        srcTexture.Load(int3(srcCoord + uint2(1, 0), 0)) +
        srcTexture.Load(int3(srcCoord + uint2(0, 1), 0)) +
        srcTexture.Load(int3(srcCoord + uint2(1, 1), 0));

    dstTexture[dstCoord] = sum * 0.25;
}
