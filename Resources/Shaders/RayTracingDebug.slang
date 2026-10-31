import Common;
import Global;
import RayTracingCommon;

ParameterBlock<GlobalShaderParameterList> globalParams;

struct RayTracingDebugViewShaderParameterList
{
    float4 bufferSizeAndInvSize;
    Bindless<RaytracingAccelerationStructure> tlas;
    Bindless<RWTexture2D> outColorTexture;
};

struct Payload
{
    float4 color;
};

CUBE_RAY_TRACING_RAY_GEN_ENTRY(
    RayTracingDebugViewRGS,
    ParameterBlock<RayTracingDebugViewShaderParameterList>, params
)
{
    CUBE_RAY_TRACING_RAY_GEN_INIT()

    const uint2 bufferPixel = DispatchRaysIndex().xy;
    const float2 bufferUV = (float2(bufferPixel) + 0.5f) / DispatchRaysDimensions().xy;
    const float2 ndc = (bufferUV - 0.5f) * float2(2.0f, -2.0f);

    float4 rayStartNDC = float4(ndc.xy, 0.0f, 1.0f);
    float4 rayEndNDC = float4(ndc.xy, 1.0f, 1.0f);

    float4 rayStartPosH = mul(rayStartNDC, globalParams.clipToWorld);
    float4 rayEndPosH = mul(rayEndNDC, globalParams.clipToWorld);

    float3 rayStartPos = rayStartPosH.xyz / rayStartPosH.w;
    float3 rayEndPos = rayEndPosH.xyz / rayEndPosH.w;

    RayDesc ray;
    ray.Origin = rayStartPos;
    ray.Direction = normalize(rayEndPos - rayStartPos);
    ray.TMin = 0.001f;
    ray.TMax = length(rayEndPos - rayStartPos);

    Payload payload;
    TraceRay(params.tlas,
        0,
        0xFF,
        0,
        1,
        0,
        ray,
        payload
    );

    params.outColorTexture[bufferPixel] = payload.color;
}

CUBE_RAY_TRACING_CLOSEST_HIT_ENTRY(
    RayTracingDebugViewCHS,
    Payload, payload,
    BuiltInTriangleIntersectionAttributes, attr
)
{
    payload.color = { 1.0f, 0.0f, 0.0f, 1.0f };
}

CUBE_RAY_TRACING_MISS_ENTRY(
    RayTracingDebugViewMS,
    Payload, payload
)
{
    payload.color = { 0.0f, 0.0f, 0.0f, 1.0f };
}
