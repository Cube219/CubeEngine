module BRDF;

import Common;

public struct BRDFParam
{
    float VoH;
    float VoN;
    float LoN;
    float NoH;
    float3 albedo;
    float metallic;
    float roughness;

    public __init(float3 L, float3 V, float3 N, float3 inAlbedo, float inMetallic, float inRoughness)
    {
        float3 H = normalize(V + L);
        VoH = dot(V, H);
        VoN = dot(V, N);
        LoN = dot(L, N);
        NoH = dot(N, H);
        albedo = inAlbedo;
        metallic = inMetallic;
        roughness = inRoughness;
    }
}

interface IDiffuse
{
    static float3 Calculate(BRDFParam param);
}

public struct Diffuse_Lambert : IDiffuse
{
    static float3 Calculate(BRDFParam param)
    {
        return param.albedo / PI;
    }
}

interface IFresnael
{
    static float3 Calculate(BRDFParam param);
}

public struct Fresnael_Schlick : IFresnael
{
    static float3 Calculate(BRDFParam param)
    {
        float3 F0 = float3(0.04);
        F0 = lerp(F0, param.albedo, param.metallic);
        
        float cosT = max(param.VoH, 0.0);
	    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosT, 0.0, 1.0), 5.0);
    }
}

interface IGeometry
{
    static float Calculate(BRDFParam param);
}

public struct Geomtry_SmithGGX : IGeometry
{
    static float Calculate(BRDFParam param)
    {
        float ggx2 = SchlickGGX(max(param.VoN, 0.0), param.roughness);
        float ggx1 = SchlickGGX(max(param.LoN, 0.0), param.roughness);
        
        return ggx1 * ggx2;
    }

    static float SchlickGGX(float NoV, float roughness)
    {
        float r = (roughness + 1.0);
        float k = (r*r) / 8.0;

        float nom = NoV;
        float denom = NoV * (1.0 - k) + k;
        
        return nom / denom;
    }
}

interface INDF
{
    static float Calculate(BRDFParam param);
}

public struct NDF_GGX : INDF
{
    static float Calculate(BRDFParam param)
    {
        float a = param.roughness * param.roughness;
        float a2 = a*a;
        float NoH = max(param.NoH, 0.0);
        float NoH2 = NoH*NoH;

        float nom = a2;
        float denom = (NoH2 * (a2 - 1.0) + 1.0);
        denom = PI * denom * denom;

        return nom / denom;
    }
}

public struct TBRDF<D, F, G, N>
    where D : IDiffuse
    where F : IFresnael
    where G : IGeometry
    where N : INDF
{
    public static float3 Calculate(BRDFParam param)
    {
        float3 f = F.Calculate(param);
        float d = N.Calculate(param);
        float g = G.Calculate(param);
        float3 nom =  d * g * f;
        float denom = 4.0 * max(param.VoN, 0.0) * max(param.LoN, 0.0) + 0.00001;
        float3 specular = nom / denom;

        float3 kS = f;
        float3 kD = float3(1.0) - kS;

        kD *= 1.0 - param.metallic;

        float3 Lo = (kD * D.Calculate(param) + specular);

        return Lo;
    }
}

public typealias BRDF = TBRDF<Diffuse_Lambert, Fresnael_Schlick, Geomtry_SmithGGX, NDF_GGX>;
