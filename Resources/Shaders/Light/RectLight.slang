implementing Light;

import BRDF;
import StaticSampler;

public struct RectLight : ILight
{
    float3 position;
    float3 direction;
    float2 rectSize;
    float3 intensity;

    float3 rectPoints[4];

    Texture2D ltc1;
    Texture2D ltc2;

    public __init(float3 position, float3 direction, float2 rectSize, float3 intensity, Texture2D ltc1, Texture2D ltc2)
    {
        this.position = position;
        this.direction = direction;
        this.rectSize = rectSize;
        this.intensity = intensity;
        this.ltc1 = ltc1;
        this.ltc2 = ltc2;
    }

    public float3 CalculateLight(float3 view, float3 normal, float3 worldPosition, MaterialValue materialValue)
    {
        float NoV = saturate(dot(normal, view));
        float2 ltcUV = float2(materialValue.roughness, sqrt(1.0f - NoV)),

        float4 ltc1V = ltc1.Sample(GetStaticLinearClampSampler(), ltcUV);
        float4 ltc2V = ltc2.Sample(GetStaticLinearClampSampler(), ltcUV);

        float3 posToLight = position - worldPosition;
        float3 L = normalize(posToLight);

        float3x3 invM = float3x3(
            float3(ltc1V.x, 0, ltc1V.y),
            float3(0, 1, 0),
            float3(ltc1V.z, 0, ltc1V.w)
        );

#ifdef MATERIAL_PBR
        float3 diffuse = CalculateLTC(view, normal, worldPosition, float3x3(1));
        float3 specular = CalculateLTC(view, normal, worldPosition, invM);

        // cube_TODO: How to calculate diffuse/specular scale?
        float3 v0 = diffuse + specular;
#else
        BRDFParam param = BRDFParam(
            L, view, normal,
            materialValue.diffuseColor, 0, 0,
            materialValue.specularColor, materialValue.shininess
        );

        const float3 diffuse = LegacyDiffuse.Calculate(param);
        const float3 specular = LegacySpecular.Calculate(param);
        const float3 Kd = LegacySpecular.DiffuseWeight(param);

        float3 v0 = Kd * diffuse + specular;
#endif

        return v0 * intensity;
    }

    float3 CalculateLTC(float3 view, float3 normal, float3 worldPosition, float3x3 invM)
    {
    }

    float IntegrateEdge(float3 v0, float3 v1, float3 n)
    {
        float x = dot(v0, v1);
        float y = abs(x);
        float a = 0.8543985 + (0.4965155 + 0.0145206*y)*y;
        float b = 3.4175940 + (4.1616724 + y)*y;
        float v = a / b;
        float theta_sintheta = (x > 0.0) ? v : 0.5*rsqrt(max(1.0 - x*x, 1e-7)) - v;
        return dot(cross(v0, v1)*theta_sintheta, n);
    }
}
