import "Light/BRDF";
import Common;
import Global;
import MonteCarlo;

struct GenerateIntegratedBRDFLUTShaderParameterList
{
    uint sampleCount;
    float width;
    Bindless<RWTexture2D> dstIntegratedBRDFLUT;
};
ParameterBlock<GenerateIntegratedBRDFLUTShaderParameterList> generateIntegratedBRDFLUTShaderParams;

float2 IntegrateBRDF(float NoV, float roughness, uint sampleCount)
{
    const float3 V = float3(sqrt(1.0 - NoV*NoV), 0, NoV);
    const float3 N = float3(0, 0, 1);

    float2 res = 0.0;
    for (uint i = 0; i < sampleCount; ++i)
    {
        const float2 Xi = Hammersley(i, sampleCount);
        const float3 H = ImportanceSampleGGX(Xi, N, roughness);
        const float3 L = normalize(2.0 * dot(V, H) * H - V);

        if (L.z > 0.0)
        {
            BRDFParam param = BRDFParam(L, V, N, 0, 0, roughness, 0, 0);
            const float VoH = max(param.VoH, 0.0);
            const float NoH = max(param.NoH, 0.0);
            const float VoN = max(param.VoN, 0.0);
            float g = Geometry_SmithGGXIBL.Calculate(param);
            float gVis = (g * VoH) / (NoH * VoN);
            float Fc = pow(1.0 - VoH, 5.0);

            res.x += (1.0 - Fc) * gVis;
            res.y += Fc * gVis;
        }
    }
    return res / sampleCount;
}

// TODO: Use entrypoint parameter and merge EnvironmentMapping files.
[shader("compute")]
[numthreads(8, 8, 1)]
void GenerateIntegratedBRDFLUTCS(
    uint3 threadID : SV_DispatchThreadID
)
{
    GenerateIntegratedBRDFLUTShaderParameterList params = generateIntegratedBRDFLUTShaderParams;

    const uint2 dstCoord = threadID.xy;
    if (any(dstCoord >= params.width.xx))
    {
        return;
    }

    const float2 uv = ((float2)(dstCoord) + 0.5) / params.width.xx;
    params.dstIntegratedBRDFLUT[dstCoord] = float4(IntegrateBRDF(uv.x, uv.y, params.sampleCount), 0);
}
