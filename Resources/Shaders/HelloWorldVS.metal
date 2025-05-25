#include <metal_stdlib>
using namespace metal;

struct SceneConstantBuffer
{
	float4x4 viewProjection;
};

struct ObjectBufferData
{
	float4x4 model;
	float4 overrideColor;
};

struct VSInput {
    float4 position [[attribute(0)]];
    float4 color [[attribute(1)]];
    float3 normal [[attribute(2)]];
};

struct PSInput
{
	float4 position [[position]];
	float4 color;
	float3 normal;
};

vertex PSInput VSMain(
	VSInput in [[stage_in]],
	constant SceneConstantBuffer& sceneBuffer [[buffer(0)]],
	constant ObjectBufferData& objectBuffer [[buffer(1)]]
) {
	PSInput result;

	result.position = sceneBuffer.viewProjection * objectBuffer.model * in.position;
	if (objectBuffer.overrideColor.a > 0)
	{
		result.color = objectBuffer.overrideColor;
	}
	else
	{
		result.color = in.color;
	}
	result.normal = in.normal;

	return result;
}
