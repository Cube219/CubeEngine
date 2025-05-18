cbuffer SceneConstantBuffer : register(b0)
{
	float4x4 viewProjection;
}

cbuffer ObjectBufferData : register(b1)
{
	float4x4 model;
	float4 overrideColor;
}

struct PSInput
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
	float3 normal : NORMAL;
};

PSInput VSMain(float4 position : POSITION, float4 color : COLOR, float3 normal : NORMAL)
{
	PSInput result;

	result.position = mul(viewProjection, mul(model, position));
	// result.position = mul(mul(position, model), viewProjection);
	if (overrideColor.a > 0)
	{
		result.color = overrideColor;
	}
	else
	{
		result.color = color;
	}
	result.normal = normal;

	return result;
}
