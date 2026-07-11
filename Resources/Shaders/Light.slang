module Light;
__include "Light/EnvironmentMapLight";
__include "Light/DirectionalLight";
__include "Light/PointLight";

import Material;

public struct PointLightInfo
{
    public float4 position;
    public float4 intensity;
};

public struct LightShaderParameterList
{
    public bool isDirectionalLightEnabled;
    public float3 directionalLightDirection;
    public float3 directionalLightIntensity;

    public uint numPointLights;
    public Bindless<StructuredBuffer<PointLightInfo>> pointLightInfos;
};

public interface ILight
{
    float3 CalculateLight(float3 view, float3 normal, float3 worldPosition, MaterialValue materialValue);
}
