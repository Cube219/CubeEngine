module Light;
__include "Light/EnvironmentMapLight";
__include "Light/DirectionalLight";
__include "Light/PointLight";
__include "Light/RectLight";

import Material;

public struct PointLightInfo
{
    public float4 position;
    public float4 intensity;
};

public struct RectLightInfo
{
    public float4 position;
    public float4 direction;
    public float4 rectSize;
    public float4 intensity;
};

public struct LightShaderParameterList
{
    public bool isDirectionalLightEnabled;
    public float3 directionalLightDirection;
    public float3 directionalLightIntensity;

    public uint numPointLights;
    public Bindless<StructuredBuffer<PointLightInfo>> pointLightInfos;

    public uint numRectLights;
    public Bindless<StructuredBuffer<RectLightInfo>> rectLightInfos;
    public Bindless<Texture2D> rectLightLTC1;
    public Bindless<Texture2D> rectLightLTC2;
};

public interface ILight
{
    float3 CalculateLight(float3 view, float3 normal, float3 worldPosition, MaterialValue materialValue);
}
