module BRDF;

import Common;

public struct BRDFParameters
{
    public float VoH;
    public float VoN;
    public float LoN;
    public float NoH;

    public __init(float3 L, float3 V, float3 N)
    {
        float3 H = normalize(V + L);
        VoH = dot(V, H);
        VoN = dot(V, N);
        LoN = dot(L, N);
        NoH = dot(N, H);
    }
}

namespace BRDF
{
    public float3 Fresnel_Schlick(float VoH, float3 F0)
    {
        float cosT = max(VoH, 0.0);
	    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosT, 0.0, 1.0), 5.0);
    }

    public float3 Fresnel_SchlickRoughness(float VoH, float3 F0, float roughness)
    {
        float cosT = max(VoH, 0.0);
	    return F0 + (max(float3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosT, 0.0, 1.0), 5.0);
    }

    public float Geometry_SchlickGGX(float VoN, float k)
    {
        float nom = VoN;
        float denom = VoN * (1.0 - k) + k;

        return nom / denom;
    }
    
    public float Geometry_SmithGGX(float VoN, float LoN, float k)
    {
        float schlickGGX1 = Geometry_SchlickGGX(max(VoN, 0.0), k);
        float schlickGGX2 = Geometry_SchlickGGX(max(LoN, 0.0), k);

        return schlickGGX1 * schlickGGX2;
    }

    public float NDF_GGX(float NoH, float a)
    {
        float a2 = a*a;
        float NoH = max(NoH, 0.0);
        float NoH2 = NoH*NoH;

        float nom = a2;
        float denom = (NoH2 * (a2 - 1.0) + 1.0);
        denom = PI * denom * denom;

        return nom / denom;
    }

    public float3 Diffuse_Lambert(float3 diffuseColor)
    {
        return diffuseColor / PI;
    }

    public float3 Specular_CookTorrance(BRDFParameters params, float3 F0, float roughness)
    {
        float3 f = BRDF::Fresnel_Schlick(params.VoH, F0);
        float d = BRDF::NDF_GGX(params.NoH, roughness * roughness);
        float g = BRDF::Geometry_SmithGGX(params.VoN, params.LoN, (roughness + 1) * (roughness + 1) / 8.0);
        float3 nom = d * g * f;
        float denom = 4.0 * max(params.VoN, 0.0) * max(params.LoN, 0.0) + 0.00001;
        return nom / denom;
    }

    public float3 Specular_BlinnPhong(BRDFParameters params, float3 specularColor, float shininess)
    {
        return specularColor * pow(max(params.NoH, 0.0), shininess);
    }
}
