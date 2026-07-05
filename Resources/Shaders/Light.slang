module Light;
__include "Light/EnvironmentMapLight";
__include "Light/DirectionalLight";

import Material;

public struct LightShaderParameterList
{
    public bool isDirectionalLightEnabled;
    public float3 directionalLightDirection;
    public float3 directionalLightIntensity;
};

public interface ILight
{
    float3 CalculateLight(float3 view, float3 normal, MaterialValue materialValue);
}
