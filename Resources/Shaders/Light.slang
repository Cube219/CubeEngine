module Light;
__include "Light/DirectionalLight";

import Material;

public interface ILight
{
    float3 CalculateLight(float3 view, float3 normal, MaterialValue materialValue);
}
