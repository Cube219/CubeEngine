module GammaCorrection;

// TODO: Use accurate version.
public float3 sRGBToLinear(float3 color)
{
    return pow(color, 2.2);
}

public float3 LinearTosRGB(float3 color)
{
    return pow(color, 1.0/2.2);
}
