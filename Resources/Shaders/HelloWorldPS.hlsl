struct PSInput
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
	float3 normal : NORMAL;
};

float4 PSMain(PSInput input) : SV_TARGET
{
	float3 lightDir = float3(1, 1, 1);
	lightDir = normalize(lightDir);

	float intensity = dot(input.normal, lightDir);

	return input.color * intensity;
}
