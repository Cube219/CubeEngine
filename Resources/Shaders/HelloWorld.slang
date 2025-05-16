cbuffer SceneConstantBuffer
{
	float4x4 viewProjection;
}

cbuffer ObjectBufferData
{
	float4x4 model;
	float4 overrideColor;
}

struct VSInput
{
	float4 position : POSITION;
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

	// output.position = mul(viewProjection, mul(model, input.position));
	output.position = mul(mul(input.position, model), viewProjection);
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
	float3 lightDir = float3(1, 1, 1);
	lightDir = normalize(lightDir);

	float intensity = dot(input.normal, lightDir);

	PSOutput output;
	output.color = input.color * intensity;

	return output;
}
