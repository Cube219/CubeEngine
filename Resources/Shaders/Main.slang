cbuffer GlobalConstantBuffer
{
	float4x4 viewProjection;
	float3 directionalLightDirection;
}

cbuffer ObjectBufferData
{
	float4x4 model;
	float4x4 modelInverse;
	float4 overrideColor;
}

struct VSInput
{
	float3 position : POSITION;
	float4 color : COLOR;
	float3 normal : NORMAL;
};

struct PSInput
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
	float3 normal : NORMAL;
};

struct PSOutput
{
	float4 color;
};

[shader("vertex")]
PSInput VSMain(VSInput input)
{
    PSInput output;

	output.position = mul(mul(float4(input.position, 1), model), viewProjection);
	if (overrideColor.a > 0)
	{
		output.color = overrideColor;
	}
	else
	{
		output.color = input.color;
	}
	output.normal = input.normal;

    return output;
}

[shader("pixel")]
PSOutput PSMain(PSInput input) : SV_TARGET
{
	float3 ligthDir = normalize(mul(directionalLightDirection, (float3x3)modelInverse));
	float intensity = dot(input.normal, ligthDir);

	PSOutput output;
	output.color = input.color * intensity;

	return output;
}
