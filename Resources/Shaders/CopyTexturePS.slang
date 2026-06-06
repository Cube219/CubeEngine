import Common;
import FullScreenVS;

struct CopyTexturePSParameterList
{
    Bindless<Texture2D> srcTexture;
    int2 size;
};
ParameterBlock<CopyTexturePSParameterList> copyTexturePSParams;

struct CopyTexturePSOutput
{
    float4 color;
};

[shader("pixel")]
CopyTexturePSOutput CopyTexturePS(FullScreenVSOutput input) : SV_TARGET
{
    CopyTexturePSParameterList params = copyTexturePSParams;

    int3 pos = int3((int2)(input.uv * params.size), 0);

    CopyTexturePSOutput output;
    output.color = params.srcTexture.Load(pos);

    return output;
}
