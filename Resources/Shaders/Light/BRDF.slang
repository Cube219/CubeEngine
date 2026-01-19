module BRDF;

import Common;

float DistributionGGX(float3 N, float3 H, float roughness)
{
	float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
	float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return nom / denom;
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
	float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

float3 fresnaelSchlick(float cosT, float3 F0)
{
	return F0 + (1.0 - F0) * pow(clamp(1.0 - cosT, 0.0, 1.0), 5.0);
}

public float3 BRDF_old(float3 L, float3 V, float3 N, float3 albedo, float metallic, float roughness)
{
	float3 H = normalize(V + L);
	
	float3 F0 = float3(0.04);
	F0 = lerp(F0, albedo, metallic);
	float3 F = fresnaelSchlick(max(dot(H, V), 0.0), F0);
	
	float D = DistributionGGX(N, H, roughness);
	float G = GeometrySmith(N, V, L, roughness);
	
	float3 nom = D * G * F;
	float denom = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.00001;
	
	float3 specular = nom / denom;
	
	float3 kS = F;
	float3 kD = float3(1.0) - kS;
	
	kD *= 1.0 - metallic;
	
	float3 Lo = (kD * (albedo / PI) + specular);

	return Lo;
}

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
