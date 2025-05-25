#include <metal_stdlib>
using namespace metal;

struct PSInput
{
	float4 position [[position]];
	float4 color;
	float3 normal;
};

fragment float4 PSMain(
	PSInput in [[stage_in]]
) {
	float3 lightDir = float3(1, 1, 1);
	lightDir = normalize(lightDir);

	float intensity = dot(in.normal, lightDir);

	return in.color * intensity;
}
