module GammaCorrection;

namespace GammaCorrection
{
    public float3 sRGBToLinear(float3 color)
    {
        return select(color > 0.04045, pow((color + 0.055) * (1.0 / 1.055), 2.4), color * (1.0 / 12.92));
    }

    public float3 LinearTosRGB(float3 color)
    {
        return select(color > 0.0031308, pow(color, 1.0 / 2.4) * 1.055 - 0.055, color * 12.92);
    }
}
